// client/network.cpp

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

NetworkManager::NetworkManager() : udpSocket(-1), tcpSocket(-1)
{
}

NetworkManager::~NetworkManager()
{
    if (udpSocket != -1)
    {
        close(udpSocket);
    }
    if (tcpSocket != -1)
    {
        close(tcpSocket);
    }
}

bool NetworkManager::connectToServer(const std::string &serverAddress, uint16_t udpPort, uint16_t tcpPort,
                                     const std::string &username, uint32_t mmr)
{
    this->serverAddress = serverAddress;
    this->udpPort = udpPort;
    this->tcpPort = tcpPort;
    this->username = username;
    this->mmr = mmr;

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1)
    {
        perror("socket");
        return false;
    }

    sockaddr_in clientAddr{};
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(udpPort);
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udpSocket, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0)
    {
        perror("bind");
        return false;
    }

    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1)
    {
        perror("socket");
        return false;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_SERVER_PORT);
    inet_pton(AF_INET, serverAddress.c_str(), &addr.sin_addr);

    if (connect(tcpSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        return false;
    }

    ConnectRequest request;
    strncpy(request.username, username.c_str(), sizeof(request.username));
    request.udpPort = udpPort;
    request.tcpPort = tcpPort;
    request.mmr = mmr;

    send(tcpSocket, &request, sizeof(request), 0);

    ConnectResponse response;
    recv(tcpSocket, &response, sizeof(response), 0);

    if (!response.success)
    {
        std::cerr << "Failed to connect to server." << std::endl;
        return false;
    }

    startListening();
    return true;
}

void NetworkManager::startListening()
{
    std::thread([this]() {
        while (true)
        {
            std::vector<uint8_t> packet = receivePacket();
            if (!packet.empty())
            {
                handlePacket(packet);
            }
        }
    }).detach();
}

void NetworkManager::handlePacket(const std::vector<uint8_t> &packet)
{
    if (packet.size() < sizeof(NetworkHeader))
        return;

    NetworkHeader *header = (NetworkHeader *)packet.data();
    if (header->type == MessageType::CONNECT_RESPONSE)
    {
        const ConnectResponse *response =
            reinterpret_cast<const ConnectResponse *>(packet.data() + sizeof(NetworkHeader));

        std::cout << "[MATCHMAKING] Opponent: " << response->opponentName << std::endl;
        std::cout << "Address: " << response->hostAddress << ", UDP: " << response->hostUdpPort
                  << ", TCP: " << response->hostTcpPort << std::endl;
        std::cout << "You are player " << (response->isPlayer1 ? "1" : "2") << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Store data / notify game
        if (onMatchFound)
            onMatchFound(*response);
    }
}

void NetworkManager::sendGameState(const GameState &state, std::string opponentUdpPort)
{
    std::vector<uint8_t> packet = createGameStatePacket(state);
    sendPacket(packet, opponentUdpPort);
}

void NetworkManager::receiveGameState(GameState &state)
{
    std::vector<uint8_t> packet = receivePacket();
    state.deserialize(packet);
}

void NetworkManager::sendChatMessage(const std::string &message, std::string opponentUdpPort)
{
    std::vector<uint8_t> packet = createChatPacket(username, message);
    sendPacket(packet, opponentUdpPort);
}

void NetworkManager::receiveChatMessage(std::string &message)
{
    std::vector<uint8_t> packet = receivePacket();

    // Ensure the packet is large enough to contain the header
    if (packet.size() < HEADER_SIZE)
    {
        std::cerr << "Received packet is too small to contain a valid header." << std::endl;
        return;
    }

    // Parse the header
    NetworkHeader header = parseHeader(packet);

    // Ensure the data size is valid
    if (header.dataSize > packet.size() - HEADER_SIZE)
    {
        std::cerr << "Received packet has an invalid data size." << std::endl;
        return;
    }

    // Construct the message string from the packet data
    message = std::string(reinterpret_cast<const char *>(packet.data() + HEADER_SIZE), header.dataSize);
}

void NetworkManager::sendPacket(const std::vector<uint8_t> &packet, std::string receivingUdpPort)
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    std::cout << "Sending packet to " << receivingUdpPort << "@wtver " << std::endl;
    addr.sin_port = htons(std::stoi(receivingUdpPort));
    inet_pton(AF_INET, serverAddress.c_str(), &addr.sin_addr);

    sendto(udpSocket, packet.data(), packet.size(), 0, (struct sockaddr *)&addr, sizeof(addr));
}

std::vector<uint8_t> NetworkManager::receivePacket()
{
    std::vector<uint8_t> packet(MAX_PACKET_SIZE);
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    std::cout << "Getting packet from wtver " << std::endl;

    int bytesReceived = recvfrom(udpSocket, packet.data(), packet.size(), 0, (struct sockaddr *)&addr, &addrlen);
    if (bytesReceived < 0)
    {
        perror("recvfrom");
        return {};
    }
    std::cout << "receved " << std::endl;

    packet.resize(bytesReceived);
    return packet;
}
} // namespace pong
