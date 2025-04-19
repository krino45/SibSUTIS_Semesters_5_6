// server/network.cpp

#include "network.h"
#include <arpa/inet.h>
#include <bits/algorithmfwd.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace pong
{

NetworkManager::NetworkManager(Matchmaker &matchmaker) : matchmaker(matchmaker), udpSocket(-1)
{
    lastUpdate = std::chrono::steady_clock::now();
}

NetworkManager::~NetworkManager()
{
    if (udpSocket != -1)
    {
        close(udpSocket);
    }
    if (acceptThread.joinable())
    {
        acceptThread.join();
    }
}

bool NetworkManager::startServer()
{
    udpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (udpSocket == -1)
    {
        perror("socket");
        return false;
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        close(udpSocket);
        return false;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(TCP_SERVER_PORT);

    if (bind(udpSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind");
        close(udpSocket);
        return false;
    }

    if (listen(udpSocket, 10) < 0)
    {
        perror("listen");
        close(udpSocket);
        return false;
    }

    std::cout << "Server started on port " << TCP_SERVER_PORT << std::endl;
    acceptThread = std::thread(&NetworkManager::acceptConnections, this);
    return true;
}

void NetworkManager::acceptConnections()
{
    while (true)
    {
        sockaddr_in clientAddr;
        socklen_t addrlen = sizeof(clientAddr);
        int clientSocket = accept(udpSocket, (struct sockaddr *)&clientAddr, &addrlen);
        if (clientSocket < 0)
        {
            perror("accept");
            continue;
        }
        std::cout << "Accepted connection on " << clientAddr.sin_port << std::endl;

        std::thread(&NetworkManager::handleClient, this, clientSocket, clientAddr).detach();
    }
}

void NetworkManager::handleClient(int clientSocket, const sockaddr_in &clientAddr)
{
    char buffer[1024];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0)
    {
        std::cerr << "Read 0 bytes\n";
        close(clientSocket);
        return;
    }

    NetworkHeader *header = reinterpret_cast<NetworkHeader *>(buffer);
    if (header->type != MessageType::CONNECT_REQUEST)
    {
        std::cerr << "Wrong header\n";
        close(clientSocket);
        return;
    }

    ConnectRequest *request = reinterpret_cast<ConnectRequest *>(buffer + sizeof(NetworkHeader));

    // Add player to matchmaker
    PlayerInfo player;
    player.username = std::string(request->username);
    player.udpPort = request->udpPort;
    player.tcpPort = request->tcpPort;
    player.mmr = request->mmr;
    player.address = inet_ntoa(clientAddr.sin_addr);

    uint8_t playerId = matchmaker.addPlayer(player);

    // Add to connected clients
    ConnectedClient client;
    client.socket = clientSocket;
    client.playerId = playerId;
    client.address = player.address;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        connectedClients.push_back(client);
    }

    // Send connection response
    ConnectResponse response;
    response.success = true;
    response.isPlayer1 = (playerId == 1);
    send(clientSocket, &response, sizeof(response), 0);

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (connectedClients.size() == 2)
        {
            serverGame.startGame();
            std::cout << "Game started with 2 players!" << std::endl;
        }
    }

    // Main client handling loop
    while (true)
    {
        bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
            break;

        header = reinterpret_cast<NetworkHeader *>(buffer);

        if (header->type == MessageType::PLAYER_INPUT)
        {
            PlayerInput *input = reinterpret_cast<PlayerInput *>(buffer + sizeof(NetworkHeader));
            serverGame.addPlayerInput(playerId, input->flags);
        }
    }

    // Cleanup - find and remove the client
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        for (auto it = connectedClients.begin(); it != connectedClients.end(); ++it)
        {
            if (it->socket == clientSocket)
            {
                connectedClients.erase(it);
                break;
            }
        }
        if (connectedClients.size() < 2 && serverGame.isActive())
        {
            serverGame.stopGame();
            std::cout << "Game stopped due to player disconnect" << std::endl;
        }
    }

    std::cout << "Handled client successfully\n";
    close(clientSocket);
}

void NetworkManager::process()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate);

    if (serverGame.isActive() && elapsed.count() >= 32)
    { // ~60fps
        serverGame.update();
        broadcastGameState();
        lastUpdate = now;
        static int counter = 0;
        if (++counter % 30 == 0)
        { // Log every second
            std::cout << "Server status - "
                      << "Clients: " << connectedClients.size()
                      << ", Game: " << (serverGame.isActive() ? "Active" : "Inactive") << std::endl;
        }
    }
}

void NetworkManager::broadcastGameState()
{
    if (!serverGame.isActive())
        return;

    serverGame.getGameState();
    std::vector<uint8_t> packet = createPacket(MessageType::GAME_STATE_UPDATE, serverGame.getGameState().frame,
                                               &serverGame.getGameState(), sizeof(GameState));
    // std::cout << "Sending clients gameState package of size " << sizeof(GameState) << ", real size: " <<
    // packet.size()
    // << "(sizeof network header: " << sizeof(NetworkHeader) << ")\n";

    std::lock_guard<std::mutex> lock(queueMutex);
    for (const auto &client : connectedClients)
    {
        send(client.socket, packet.data(), packet.size(), 0);
    }
}

} // namespace pong