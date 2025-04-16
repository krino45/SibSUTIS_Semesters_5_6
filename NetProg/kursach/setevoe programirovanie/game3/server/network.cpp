// server/network.cpp

#include "network.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace pong
{

NetworkManager::NetworkManager(Matchmaker &matchmaker) : matchmaker(matchmaker), serverSocket(-1)
{
}

NetworkManager::~NetworkManager()
{
    if (serverSocket != -1)
    {
        close(serverSocket);
    }
    if (acceptThread.joinable())
    {
        acceptThread.join();
    }
}

bool NetworkManager::startServer()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        perror("socket");
        return false;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(TCP_SERVER_PORT);

    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind");
        close(serverSocket);
        return false;
    }

    if (listen(serverSocket, 10) < 0)
    {
        perror("listen");
        close(serverSocket);
        return false;
    }

    acceptThread = std::thread(&NetworkManager::acceptConnections, this);
    return true;
}

void NetworkManager::acceptConnections()
{
    while (true)
    {
        sockaddr_in clientAddr;
        socklen_t addrlen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrlen);
        if (clientSocket < 0)
        {
            perror("accept");
            continue;
        }

        std::thread(&NetworkManager::handleClient, this, clientSocket, clientAddr).detach();
    }
}

void NetworkManager::handleClient(int clientSocket, const sockaddr_in &clientAddr)
{
    char buffer[1024];
    int bytesRead = read(clientSocket, buffer, sizeof(buffer));
    if (bytesRead <= 0)
    {
        close(clientSocket);
        return;
    }

    ConnectRequest request;
    memcpy(&request, buffer, sizeof(request));

    PlayerInfo player;
    player.username = std::string(request.username);
    player.udpPort = request.udpPort;
    player.tcpPort = request.tcpPort;
    player.mmr = request.mmr;
    player.address = inet_ntoa(clientAddr.sin_addr);

    matchmaker.addPlayer(player);

    // Send response to client
    ConnectResponse response;
    response.success = true;
    send(clientSocket, &response, sizeof(response), 0);

    close(clientSocket);
}

void NetworkManager::process()
{
    // Handle network messages and update game state
}

} // namespace pong
