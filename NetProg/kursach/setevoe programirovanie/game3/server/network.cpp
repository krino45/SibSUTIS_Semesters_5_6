// server/network.cpp
#include "network.h"
#include "matchmaker.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

namespace pong
{

NetworkManager::NetworkManager() : udpSocket(-1), running(false), matchmaker(nullptr), gameManager(nullptr)
{
    lastUpdate = std::chrono::steady_clock::now();
    lastCleanupTime = std::chrono::steady_clock::now();
}

NetworkManager::~NetworkManager()
{
    shutdown();
}

bool NetworkManager::startServer(uint16_t port)
{
    // Create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0)
    {
        perror("Failed to create socket");
        return false;
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        close(udpSocket);
        udpSocket = -1;
        return false;
    }

    // Bind to port
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(udpSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Bind failed");
        close(udpSocket);
        udpSocket = -1;
        return false;
    }

    // Set non-blocking mode
    int flags = fcntl(udpSocket, F_GETFL, 0);
    fcntl(udpSocket, F_SETFL, flags | O_NONBLOCK);

    // Start receiver thread
    running = true;
    receiveThread = std::thread(&NetworkManager::receiveLoop, this);

    std::cout << "UDP Server started on port " << port << std::endl;
    return true;
}

void NetworkManager::shutdown()
{
    running = false;

    if (receiveThread.joinable())
    {
        receiveThread.join();
    }

    if (udpSocket >= 0)
    {
        close(udpSocket);
        udpSocket = -1;
    }
}

void NetworkManager::receiveLoop()
{
    const size_t bufferSize = 2048;
    std::vector<uint8_t> buffer(bufferSize);
    sockaddr_in senderAddr{};
    socklen_t senderLen = sizeof(senderAddr);

    while (running)
    {
        // Receive data
        senderLen = sizeof(senderAddr);
        ssize_t bytesReceived = recvfrom(udpSocket, buffer.data(), bufferSize, 0, (sockaddr *)&senderAddr, &senderLen);

        if (bytesReceived > 0)
        {
            // Process valid packets
            std::vector<uint8_t> packetData(buffer.begin(), buffer.begin() + bytesReceived);
            handlePacket(packetData, senderAddr);
        }
        else if (bytesReceived < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
        {
            // Only log actual errors, not would-block conditions
            perror("Error receiving data");
        }

        // Small sleep to prevent CPU hogging in the loop
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void NetworkManager::handlePacket(const std::vector<uint8_t> &data, const sockaddr_in &sender)
{
    // Check if packet is large enough for a header
    if (data.size() < sizeof(NetworkHeader))
    {
        std::cerr << "Received malformed packet (too small for header)" << std::endl;
        return;
    }

    // Parse header
    const NetworkHeader *header = reinterpret_cast<const NetworkHeader *>(data.data());
    std::string clientId = getClientIdentifier(sender);

    // Handle packet based on message type
    switch (header->type)
    {
    case MessageType::CONNECT_REQUEST:
        handleConnectRequest(data, sender);
        break;

    case MessageType::PLAYER_INPUT:
        handlePlayerInput(data, clientId);
        break;

    default:
        std::cerr << "Received unhandled message type: " << static_cast<int>(header->type) << std::endl;
        break;
    }
}

void NetworkManager::handleConnectRequest(const std::vector<uint8_t> &data, const sockaddr_in &sender)
{
    // Check packet size
    if (data.size() < sizeof(NetworkHeader) + sizeof(ConnectRequest))
    {
        std::cerr << "Connect request packet too small" << std::endl;
        return;
    }

    // Parse connect request
    const NetworkHeader *header = reinterpret_cast<const NetworkHeader *>(data.data());
    const ConnectRequest *request = reinterpret_cast<const ConnectRequest *>(data.data() + sizeof(NetworkHeader));

    std::string clientId = getClientIdentifier(sender);
    std::string clientAddr = inet_ntoa(sender.sin_addr);
    uint16_t clientPort = ntohs(sender.sin_port);

    std::cout << "Received connection request from " << request->username << " at " << clientAddr << ":" << clientPort
              << std::endl;

    // Create player info for matchmaking
    PlayerInfo player;
    player.username = request->username;
    player.clientId = clientId;
    player.address = clientAddr;
    player.udpPort = request->udpPort; // Client's listening port
    player.tcpPort = request->tcpPort; // For direct chat
    player.mmr = request->mmr;

    // Register player with matchmaker
    uint8_t playerId = 0;
    if (matchmaker)
    {
        playerId = matchmaker->registerPlayer(player);
    }

    // Add client to our connected clients
    {
        std::lock_guard<std::mutex> lock(clientsMutex);

        // Check if client already exists
        auto it = clientIdToIndex.find(clientId);
        if (it != clientIdToIndex.end())
        {
            // Update existing client
            clients[it->second].lastActivityTime =
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                    .count();
        }
        else
        {
            // Add new client
            ConnectedClient newClient;
            newClient.clientId = clientId;
            newClient.playerId = playerId;
            newClient.address = clientAddr;
            newClient.port = request->udpPort; // Use client's listening port, not the source port
            newClient.lastActivityTime =
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                    .count();

            clients.push_back(newClient);
            clientIdToIndex[clientId] = clients.size() - 1;
        }
    }

    // Send acknowledgment response
    ConnectResponse response;
    memset(&response, 0, sizeof(response)); // Zero out entire struct
    strncpy(response.hostAddress, player.address.data(), player.address.size());
    response.hostTcpPort = player.tcpPort;
    response.hostUdpPort = player.udpPort;
    response.opponentName[0] = '\0';
    response.success = true;
    response.isPlayer1 = (playerId == 1);

    std::vector<uint8_t> responsePacket = createPacket(MessageType::CONNECT_RESPONSE, 0, &response, sizeof(response));

    // Send to client's listening port, not the source port
    sendToClient(clientAddr, request->udpPort, responsePacket);
}

void NetworkManager::handlePlayerInput(const std::vector<uint8_t> &data, const std::string &clientId)
{
    // Check packet size
    if (data.size() < sizeof(NetworkHeader) + sizeof(PlayerInput))
    {
        std::cerr << "Player input packet too small" << std::endl;
        return;
    }

    // Parse player input
    const NetworkHeader *header = reinterpret_cast<const NetworkHeader *>(data.data());
    const PlayerInput *input = reinterpret_cast<const PlayerInput *>(data.data() + sizeof(NetworkHeader));

    // Find player ID from client ID
    uint8_t playerId = 0;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        auto it = clientIdToIndex.find(clientId);
        if (it != clientIdToIndex.end())
        {
            playerId = clients[it->second].playerId;
            // Update last activity time
            clients[it->second].lastActivityTime =
                std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
                    .count();
        }
    }

    // Find which game this client is in
    uint32_t gameId = gameManager->findGameIdForClient(clientId);
    if (gameId == 0)
    {
        std::cerr << "Client not in any game: " << clientId << std::endl;
        return;
    }

    GameInstance *game = gameManager->getGame(gameId);
    if (!game)
    {
        std::cerr << "Game not found: " << gameId << std::endl;
        return;
    }

    if (input->flags == InputFlags::QUIT)
    {
        handleClientDisconnect(clientId, true);
    }
    else
    {
        game->addPlayerInput(playerId, input->flags);
    }
}

uint32_t NetworkManager::findGameIdForClient(const std::string &clientId)
{
    if (gameManager)
    {
        return gameManager->findGameIdForClient(clientId);
    }
    return 0;
}

void NetworkManager::handleClientDisconnect(const std::string &clientId, bool notifyOthers)
{
    // First check if client exists to avoid unnecessary work
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        auto it = clientIdToIndex.find(clientId);
        if (it == clientIdToIndex.end())
            return;
    }

    // Get game info first before modifying any data structures
    uint32_t gameId = gameManager->findGameIdForClient(clientId);
    GameInstance *game = gameManager->getGame(gameId);

    // Create disconnect packet once
    std::vector<uint8_t> packet = createPacket(MessageType::DISCONNECT_EVENT, 0, nullptr, 0);

    // Collect other players that need to be disconnected
    std::vector<std::string> otherPlayersToDisconnect;
    if (notifyOthers && game)
    {
        for (const std::string &otherClientId : game->getAllPlayers())
        {
            if (otherClientId != clientId)
            {
                otherPlayersToDisconnect.push_back(otherClientId);
            }
        }
    }

    // Notify other players
    for (const std::string &otherClientId : otherPlayersToDisconnect)
    {
        std::cout << "Disconnecting remaining player in game " << gameId << ": " << otherClientId << std::endl;
        sendToClient(otherClientId, packet);
    }

    // Now remove the current client
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        auto it = clientIdToIndex.find(clientId);
        if (it == clientIdToIndex.end())
            return;

        const ConnectedClient &disconnected = clients[it->second];
        std::cout << "Client " << clientId << " disconnected: " << disconnected.address << std::endl;

        // Remove this client from list
        clients.erase(clients.begin() + it->second);
        clientIdToIndex.erase(it);

        // Rebuild index
        clientIdToIndex.clear();
        for (size_t i = 0; i < clients.size(); ++i)
        {
            clientIdToIndex[clients[i].clientId] = i;
        }
    }

    // Handle game cleanup
    if (game)
    {
        game->stopGame();
        gameManager->removeGame(gameId);
    }

    // Finally, recursively disconnect other players, but after we've already
    // finished processing the current client
    for (const std::string &otherClientId : otherPlayersToDisconnect)
    {
        handleClientDisconnect(otherClientId, false);
    }
}
void NetworkManager::process()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate);

    if (elapsed.count() >= 16)
    { // ~60fps
        gameManager->updateAllGames();
        lastUpdate = now;
    }
}

void NetworkManager::sendToClient(const std::string &clientId, const std::vector<uint8_t> &packet)
{
    std::lock_guard<std::mutex> lock(clientsMutex);

    auto it = clientIdToIndex.find(clientId);
    if (it != clientIdToIndex.end())
    {
        const ConnectedClient &client = clients[it->second];
        sendToClient(client.address, client.port, packet);
    }
    else
    {
        std::cerr << "Attempted to send to unknown client ID: " << clientId << std::endl;
    }
}

void NetworkManager::sendToClient(const std::string &address, uint16_t port, const std::vector<uint8_t> &packet)
{
    sockaddr_in clientAddr{};
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, address.c_str(), &clientAddr.sin_addr) <= 0)
    {
        std::cerr << "Invalid client address: " << address << std::endl;
        return;
    }

    ssize_t bytesSent = sendto(udpSocket, packet.data(), packet.size(), 0, (sockaddr *)&clientAddr, sizeof(clientAddr));

    if (bytesSent < 0)
    {
        perror("Failed to send packet");
    }
    else if (bytesSent != packet.size())
    {
        std::cerr << "Sent " << bytesSent << " bytes, expected " << packet.size() << std::endl;
    }
}

void NetworkManager::broadcastToGame(const std::vector<uint8_t> &packet, uint32_t gameId)
{
    std::lock_guard<std::mutex> lock(clientsMutex);

    auto *game = gameManager->getGame(gameId);
    if (!game)
        return;

    for (std::string id : game->getAllPlayers())
    {
        sendToClient(id, packet);
    }
}

std::string NetworkManager::getClientIdentifier(const sockaddr_in &addr)
{
    return std::string(inet_ntoa(addr.sin_addr)) + ":" + std::to_string(ntohs(addr.sin_port));
}

} // namespace pong