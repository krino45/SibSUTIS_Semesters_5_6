// server/matchmaker.cpp
#include "matchmaker.h"
#include "network.h"
#include <iostream>

namespace pong
{

Matchmaker::Matchmaker() : networkManager(nullptr), gameManager(nullptr), currentPlayer1(""), currentPlayer2("")
{
}

Matchmaker::~Matchmaker()
{
}

void Matchmaker::loadMMR()
{
    std::ifstream file(mmrFile);
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string name;
        int mmr;
        if (std::getline(ss, name, ',') && ss >> mmr)
        {
            mmrMap[name] = mmr;
        }
    }
}

void Matchmaker::saveMMR()
{
    std::ofstream file(mmrFile);
    for (const auto &[user, mmr] : mmrMap)
    {
        file << user << "," << mmr << "\n";
    }
}

void Matchmaker::updateMMR(const std::string &winner, const std::string &loser)
{
    int K = 32;
    int Ra = mmrMap[winner];
    int Rb = mmrMap[loser];

    float Ea = 1.0f / (1.0f + pow(10.0f, (Rb - Ra) / 400.0f));
    float Eb = 1.0f / (1.0f + pow(10.0f, (Ra - Rb) / 400.0f));

    mmrMap[winner] = std::round(Ra + K * (1 - Ea));
    mmrMap[loser] = std::round(Rb + K * (0 - Eb));

    saveMMR();
}

uint8_t Matchmaker::registerPlayer(const PlayerInfo &player)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    // Check if player is already registered
    if (activePlayersByUsername.find(player.username) != activePlayersByUsername.end() ||
        activePlayersByClientId.find(player.clientId) != activePlayersByClientId.end())
    {
        std::cerr << "Player already registered: " << player.username << std::endl;
        return 0;
    }

    if (mmrMap.find(player.username) == mmrMap.end())
    {
        mmrMap[player.username] = 1000;
    }

    activePlayersByUsername[player.username] = player;
    activePlayersByClientId[player.clientId] = player;

    waitingPlayers.push(player);

    return waitingPlayers.size();
}

void Matchmaker::process()
{
    PlayerInfo player1, player2;
    if (findMatch(player1, player2))
    {
        std::cout << "Match found: " << player1.username << "(" << mmrMap[player1.username] << ") vs "
                  << player2.username << "(" << mmrMap[player2.username] << ")" << std::endl;

        // Store current players
        {
            std::lock_guard<std::mutex> lock(playersMutex);
            currentPlayer1 = player1.username;
            currentPlayer2 = player2.username;
        }

        notifyPlayersAboutMatch(player1, player2);
    }
}

bool Matchmaker::findMatch(PlayerInfo &player1, PlayerInfo &player2)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    if (waitingPlayers.size() < 2)
        return false;

    std::vector<PlayerInfo> queue;
    while (!waitingPlayers.empty())
    {
        queue.push_back(waitingPlayers.front());
        waitingPlayers.pop();
    }

    const int maxDeltaStart = 100;
    const int maxDeltaLimit = 600;

    for (int delta = maxDeltaStart; delta <= maxDeltaLimit; delta += 100)
    {
        for (size_t i = 0; i < queue.size(); ++i)
        {
            int mmr1 = mmrMap[queue[i].username];
            for (size_t j = i + 1; j < queue.size(); ++j)
            {
                int mmr2 = mmrMap[queue[j].username];
                if (std::abs(mmr1 - mmr2) <= delta)
                {
                    player1 = queue[i];
                    player2 = queue[j];

                    // Remove both from queue
                    queue.erase(queue.begin() + j);
                    queue.erase(queue.begin() + i);

                    // Push remaining back into waiting queue
                    for (const auto &p : queue)
                        waitingPlayers.push(p);

                    return true;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    // Put all players back if no match
    for (const auto &p : queue)
        waitingPlayers.push(p);

    return false;
}

void Matchmaker::notifyPlayersAboutMatch(const PlayerInfo &player1, const PlayerInfo &player2)
{
    if (!networkManager || !gameManager)
    {
        std::cerr << "NetworkManager not set in Matchmaker" << std::endl;
        return;
    }

    uint32_t gameId =
        gameManager->createGame(player1.clientId, player2.clientId, false); // not starting the game to desync it a bit

    // Create and send match notification for player 1
    std::vector<uint8_t> packet1 = createMatchNotificationPacket(player1, player2);
    networkManager->sendToClient(player1.address, player1.udpPort, packet1);

    // Create and send match notification for player 2
    std::vector<uint8_t> packet2 = createMatchNotificationPacket(player2, player1);
    networkManager->sendToClient(player2.address, player2.udpPort, packet2);

    std::cout << "Sent match notifications to both players" << std::endl;

    if (GameInstance *game = gameManager->getGame(gameId))
    {
        std::thread([gameId, this] {
            std::this_thread::sleep_for(std::chrono::seconds(5));

            // Re-acquire the game pointer after delay
            if (GameInstance *gameAfterDelay = gameManager->getGame(gameId))
            {
                gameAfterDelay->startGame();
            }
        }).detach();
    }
}

std::string Matchmaker::getPlayer1Name()
{
    std::lock_guard<std::mutex> lock(playersMutex);
    return currentPlayer1;
}

std::string Matchmaker::getPlayer2Name()
{
    std::lock_guard<std::mutex> lock(playersMutex);
    return currentPlayer2;
}

void Matchmaker::deregisterPlayer(const std::string &username)
{

    std::lock_guard<std::mutex> lock(queueMutex);
    auto it = activePlayersByUsername.find(username);
    if (it != activePlayersByClientId.end())
    {
        const std::string &clientId = it->second.clientId;
        activePlayersByUsername.erase(username);
        activePlayersByClientId.erase(clientId);

        // Check if this was one of the current players
        std::lock_guard<std::mutex> plock(playersMutex);
        if (username == currentPlayer1)
        {
            currentPlayer1.clear();
        }
        if (username == currentPlayer2)
        {
            currentPlayer2.clear();
        }
    }

    // Remove from waiting queue if present
    std::queue<PlayerInfo> tempQueue;
    while (!waitingPlayers.empty())
    {
        PlayerInfo player = waitingPlayers.front();
        waitingPlayers.pop();
        if (player.username != username)
        {
            tempQueue.push(player);
        }
    }
    waitingPlayers = tempQueue;
}

void Matchmaker::handlePlayerDisconnect(const std::string &clientId)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    auto it = activePlayersByClientId.find(clientId);
    if (it != activePlayersByClientId.end())
    {
        std::string user = it->second.username;

        // Consider them the loser, find the other player
        std::string winner;
        {
            std::lock_guard<std::mutex> lock(playersMutex);
            if (user == currentPlayer1)
                winner = currentPlayer2;
            else if (user == currentPlayer2)
                winner = currentPlayer1;
        }

        if (!winner.empty())
        {
            updateMMR(winner, user);
            std::cout << "MMR updated due to disconnect: " << winner << " beat " << user << std::endl;
        }
    }

    if (it != activePlayersByClientId.end())
    {
        const std::string &username = it->second.username;
        activePlayersByUsername.erase(username);
        activePlayersByClientId.erase(clientId);

        // Check if this was one of the current players
        std::lock_guard<std::mutex> plock(playersMutex);
        if (username == currentPlayer1)
        {
            currentPlayer1.clear();
        }
        if (username == currentPlayer2)
        {
            currentPlayer2.clear();
        }
    }

    // Remove from waiting queue if present
    std::queue<PlayerInfo> tempQueue;
    while (!waitingPlayers.empty())
    {
        PlayerInfo player = waitingPlayers.front();
        waitingPlayers.pop();
        if (player.clientId != clientId)
        {
            tempQueue.push(player);
        }
    }
    waitingPlayers = tempQueue;
}

std::vector<uint8_t> Matchmaker::createMatchNotificationPacket(const PlayerInfo &player, const PlayerInfo &opponent)
{
    // Create response structure
    ConnectResponse response;
    response.success = true;
    memset(&response, 0, sizeof(response));
    strncpy(response.opponentName, opponent.username.data(), opponent.username.size());
    strncpy(response.hostAddress, opponent.address.data(), opponent.address.size());

    response.hostUdpPort = opponent.udpPort;
    response.hostTcpPort = opponent.tcpPort;

    // Arbitrary player order determination
    response.isPlayer1 = (player.username < opponent.username);

    response.success = true;

    // Create packet
    return createPacket(MessageType::CONNECT_RESPONSE, 0, &response, sizeof(response));
}

} // namespace pong