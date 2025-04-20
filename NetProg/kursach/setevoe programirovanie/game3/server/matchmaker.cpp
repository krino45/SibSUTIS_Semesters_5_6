// server/matchmaker.cpp
#include "matchmaker.h"
#include "network.h"
#include <iostream>

namespace pong
{

Matchmaker::Matchmaker() : networkManager(nullptr), currentPlayer1(""), currentPlayer2("")
{
}

Matchmaker::~Matchmaker()
{
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

    // Store player info in both maps for quick lookups
    activePlayersByUsername[player.username] = player;
    activePlayersByClientId[player.clientId] = player;

    // Add to waiting queue
    waitingPlayers.push(player);

    // Return player position as temporary ID
    return waitingPlayers.size();
}

void Matchmaker::process()
{
    PlayerInfo player1, player2;
    if (findMatch(player1, player2))
    {
        std::cout << "Match found: " << player1.username << " vs " << player2.username << std::endl;

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

    if (waitingPlayers.size() >= 2)
    {
        player1 = waitingPlayers.front();
        waitingPlayers.pop();

        player2 = waitingPlayers.front();
        waitingPlayers.pop();

        return true;
    }

    return false;
}

void Matchmaker::notifyPlayersAboutMatch(const PlayerInfo &player1, const PlayerInfo &player2)
{
    if (!networkManager)
    {
        std::cerr << "NetworkManager not set in Matchmaker" << std::endl;
        return;
    }

    // Create and send match notification for player 1
    std::vector<uint8_t> packet1 = createMatchNotificationPacket(player1, player2);
    networkManager->sendToClient(player1.address, player1.udpPort, packet1);

    // Create and send match notification for player 2
    std::vector<uint8_t> packet2 = createMatchNotificationPacket(player2, player1);
    networkManager->sendToClient(player2.address, player2.udpPort, packet2);

    std::cout << "Sent match notifications to both players" << std::endl;
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

void Matchmaker::handlePlayerDisconnect(const std::string &clientId)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    // Remove from active players maps
    auto clientIt = activePlayersByClientId.find(clientId);
    if (clientIt != activePlayersByClientId.end())
    {
        const std::string &username = clientIt->second.username;
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
    strncpy(response.opponentName, opponent.username.c_str(), sizeof(response.opponentName) - 1);
    response.opponentName[sizeof(response.opponentName) - 1] = '\0';

    strncpy(response.hostAddress, opponent.address.c_str(), sizeof(response.hostAddress) - 1);
    response.hostAddress[sizeof(response.hostAddress) - 1] = '\0';

    response.hostUdpPort = opponent.udpPort;
    response.hostTcpPort = opponent.tcpPort;

    // Arbitrary player order determination
    response.isPlayer1 = (player.username < opponent.username);

    // Create packet
    return createPacket(MessageType::CONNECT_RESPONSE, 0, &response, sizeof(response));
}

} // namespace pong