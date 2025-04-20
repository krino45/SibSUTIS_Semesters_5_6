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

NetworkManager::NetworkManager() : udpSocket(-1), running(false), matchmaker(nullptr), serverGame(nullptr)
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

size_t NetworkManager::getClientCount() const
{
    return clients.size();
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
    response.success = true;
    response.isPlayer1 = (playerId == 1);

    std::vector<uint8_t> responsePacket = createPacket(MessageType::CONNECT_RESPONSE, 0, &response, sizeof(response));

    // Send to client's listening port, not the source port
    sendToClient(clientAddr, request->udpPort, responsePacket);

    // Check if we can start a game
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        if (clients.size() == 2)
        {
            serverGame->startGame();
            std::cout << "Game started with 2 players!" << std::endl;
        }
    }
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

    // Process input if player is valid
    if (playerId > 0 && serverGame)
    {
        if (input->flags == InputFlags::QUIT)
        {
            handleClientDisconnect(clientId, true);
            std::lock_guard<std::mutex> lock(clientsMutex);
            auto it = clientIdToIndex.find(clientId);
            if (it != clientIdToIndex.end())
            {
                // Remove client
                clients.erase(clients.begin() + it->second);
                clientIdToIndex.erase(it);

                // Rebuild index map
                clientIdToIndex.clear();
                for (size_t i = 0; i < clients.size(); ++i)
                {
                    clientIdToIndex[clients[i].clientId] = i;
                }

                // Stop game if there aren't enough players
                if (clients.size() < 2 && serverGame->isActive())
                {
                    serverGame->stopGame();
                }
            }
            matchmaker->handlePlayerDisconnect(clientId);

            return;
        }
        else
        {
            serverGame->addPlayerInput(playerId, input->flags);
        }
    }
    else
    {
        std::cerr << "Received input from unknown client: " << clientId << std::endl;
    }
}

void NetworkManager::handleClientDisconnect(const std::string &clientId, bool notifyOthers)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    auto it = clientIdToIndex.find(clientId);
    if (it != clientIdToIndex.end())
    {
        // Create disconnect notification packet
        std::vector<uint8_t> packet = createPacket(MessageType::DISCONNECT_EVENT, 0, nullptr, 0);

        // Remove client
        clients.erase(clients.begin() + it->second);
        clientIdToIndex.erase(it);

        // Rebuild index map
        clientIdToIndex.clear();
        for (size_t i = 0; i < clients.size(); ++i)
        {
            clientIdToIndex[clients[i].clientId] = i;
        }

        // Notify remaining clients if requested
        if (notifyOthers && !clients.empty())
        {
            broadcastToAllClients(packet);
        }

        // Stop game if there aren't enough players
        if (clients.size() < 2 && serverGame && serverGame->isActive())
        {
            serverGame->stopGame();
        }
    }
}

void NetworkManager::process()
{
    auto now = std::chrono::steady_clock::now();

    // Process game updates
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate);
    if (serverGame && serverGame->isActive() && elapsed.count() >= 16)
    { // ~60fps
        serverGame->update();
        broadcastGameState();
        lastUpdate = now;

        // Periodic status log
        static int counter = 0;
        if (++counter % 3000 == 0)
        {
            std::cout << "Server status - Clients: " << getClientCount()
                      << ", Game: " << (serverGame->isActive() ? "Active" : "Inactive") << std::endl;
        }
    }

    // Periodic cleanup of inactive clients (every 30 seconds)
    auto cleanupElapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastCleanupTime);
    if (cleanupElapsed.count() >= 30)
    {
        cleanupInactiveClients();
        lastCleanupTime = now;
    }
}

void NetworkManager::cleanupInactiveClients()
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    auto currentTime =
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    // Check each client for inactivity
    for (auto it = clients.begin(); it != clients.end();)
    {
        if (currentTime - it->lastActivityTime > 30) // 30 seconds timeout
        {
            std::cout << "Disconnecting inactive client: " << it->clientId << std::endl;
            handleClientDisconnect(it->clientId, true);
            it = clients.begin(); // Start over as vector was modified
        }
        else
        {
            ++it;
        }
    }
}

void NetworkManager::broadcastGameState()
{
    if (!serverGame || !serverGame->isActive())
    {
        return;
    }

    // Create game state packet
    std::vector<uint8_t> packet = createPacket(MessageType::GAME_STATE_UPDATE, serverGame->getGameState().frame,
                                               &serverGame->getGameState(), sizeof(GameState));

    // Send to all clients
    broadcastToAllClients(packet);
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

void NetworkManager::broadcastToAllClients(const std::vector<uint8_t> &packet)
{
    std::lock_guard<std::mutex> lock(clientsMutex);

    for (const auto &client : clients)
    {
        sendToClient(client.address, client.port, packet);
    }
}

std::string NetworkManager::getClientIdentifier(const sockaddr_in &addr)
{
    return std::string(inet_ntoa(addr.sin_addr)) + ":" + std::to_string(ntohs(addr.sin_port));
}

} // namespace pong