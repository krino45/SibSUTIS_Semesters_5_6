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

    // Create UDP socket first
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1)
    {
        perror("udp socket");
        return false;
    }

    // Bind UDP socket
    sockaddr_in clientAddr{};
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(udpPort);
    clientAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udpSocket, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) < 0)
    {
        perror("udp bind");
        close(udpSocket);
        return false;
    }

    // Create TCP socket with timeout
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1)
    {
        perror("tcp socket");
        close(udpSocket);
        return false;
    }

    // Set timeout for connection attempt (5 seconds)
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(tcpSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(tcpSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_SERVER_PORT);

    if (inet_pton(AF_INET, serverAddress.c_str(), &addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid server address: " << serverAddress << std::endl;
        close(udpSocket);
        close(tcpSocket);
        return false;
    }

    // Try to connect with retries
    int retries = 3;
    while (retries-- > 0)
    {
        if (connect(tcpSocket, (struct sockaddr *)&addr, sizeof(addr)) == 0)
        {
            break; // Connection successful
        }

        std::cerr << "Connection attempt failed (" << retries << " retries left)" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (retries <= 0)
    {
        std::cerr << "Failed to connect to server after multiple attempts" << std::endl;
        close(udpSocket);
        close(tcpSocket);
        return false;
    }

    // Prepare connection request
    ConnectRequest request;
    strncpy(request.username, username.c_str(), sizeof(request.username));

    request.udpPort = udpPort;
    request.tcpPort = tcpPort;
    request.mmr = mmr;

    std::vector<uint8_t> packet = createPacket(MessageType::CONNECT_REQUEST, 0, &request, sizeof(request));

    // Send connection request
    if (send(tcpSocket, packet.data(), packet.size(), 0) != packet.size())
    {
        perror("send");
        close(udpSocket);
        close(tcpSocket);
        return false;
    }

    // Receive response with timeout
    ConnectResponse response;
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(tcpSocket, &readSet);

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    ssize_t bytesReceived = recv(tcpSocket, &response, sizeof(response), 0);
    if (bytesReceived < 0)
    {
        perror("recv");
        close(udpSocket);
        close(tcpSocket);
        return false;
    }
    else if (bytesReceived == 0)
    {
        std::cerr << "Connection closed by server" << std::endl;
        close(udpSocket);
        close(tcpSocket);
        return false;
    }
    else if (bytesReceived != sizeof(response))
    {
        std::cerr << "Received unexpected number of bytes: " << bytesReceived << "want: " << sizeof(response)
                  << std::endl;
        close(udpSocket);
        close(tcpSocket);
        return false;
    }

    std::cout << "Successfully connected to server" << std::endl;
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
        isPlayer1 = response->isPlayer1;
        std::this_thread::sleep_for(std::chrono::seconds(2));

        // Store data / notify game
        if (onMatchFound)
            onMatchFound(*response);
    }
}

void NetworkManager::sendPlayerInput(uint8_t inputFlags, uint32_t currentFrame)
{
    PlayerInput input;
    input.playerId = isPlayer1 ? 1 : 2; // Add this to NetworkManager class
    input.flags = inputFlags;
    input.frameNumber = currentFrame;
    if (input.flags == 0)
    {
        return;
    }

    std::vector<uint8_t> packet = createInputPacket(input);
    if (send(tcpSocket, packet.data(), packet.size(), 0) != packet.size())
    {
        std::cerr << "Failed to send input packet" << std::endl;
    }
}

bool NetworkManager::receiveGameState(GameState &state)
{
    std::vector<uint8_t> buffer(sizeof(NetworkHeader) + sizeof(GameState));
    int bytesReceived = recv(tcpSocket, buffer.data(), buffer.size(), MSG_DONTWAIT);

    if (bytesReceived > 0)
    {
        // Verify minimum packet size
        if (bytesReceived < sizeof(NetworkHeader))
        {
            std::cerr << "Packet too small for header" << std::endl;
            return false;
        }

        NetworkHeader *header = reinterpret_cast<NetworkHeader *>(buffer.data());

        // Handle only game state updates
        if (header->type == MessageType::GAME_STATE_UPDATE)
        {
            if (bytesReceived != sizeof(NetworkHeader) + sizeof(GameState))
            {
                std::cerr << "Invalid game state packet size" << std::endl;
                return false;
            }
            return state.deserialize(std::vector<uint8_t>(buffer.begin() + sizeof(NetworkHeader), buffer.end()));
        }
        // Silently ignore other packet types
    }
    return false;
}

std::vector<uint8_t> NetworkManager::createInputPacket(const PlayerInput &input)
{
    std::vector<uint8_t> packet(sizeof(NetworkHeader) + sizeof(PlayerInput));
    NetworkHeader *header = reinterpret_cast<NetworkHeader *>(packet.data());

    header->type = MessageType::PLAYER_INPUT;
    header->frame = input.frameNumber;
    header->dataSize = sizeof(PlayerInput);

    memcpy(packet.data() + sizeof(NetworkHeader), &input, sizeof(PlayerInput));
    return packet;
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

bool NetworkManager::isConnected()
{
    return tcpSocket != -1 && udpSocket != -1;
}

} // namespace pong
