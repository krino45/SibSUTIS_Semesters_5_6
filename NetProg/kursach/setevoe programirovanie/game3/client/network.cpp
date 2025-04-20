// client/network.cpp

#include "network.h"

namespace pong
{

NetworkManager::NetworkManager() : udpSocket(-1)
{
}

NetworkManager::~NetworkManager()
{
    if (udpSocket != -1)
    {
        close(udpSocket);
    }
}

bool NetworkManager::connectToServer(const std::string &serverAddress, uint16_t udpPort, uint16_t tcpPort,
                                     const std::string &username, uint32_t mmr)
{
    this->serverAddress = serverAddress;
    this->udpPort = udpPort;
    this->username = username;
    this->mmr = mmr;

    // Create UDP socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1)
    {
        perror("udp socket");
        return false;
    }

    // Bind UDP socket to receive messages
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

    // Set up server address for sending
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_SERVER_PORT);

    if (inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr) <= 0)
    {
        std::cerr << "Invalid server address: " << serverAddress << std::endl;
        close(udpSocket);
        return false;
    }

    // Prepare connection request
    ConnectRequest request;
    strncpy(request.username, username.c_str(), sizeof(request.username));
    request.udpPort = udpPort;
    request.tcpPort = tcpPort; // For direct player-to-player chat
    request.mmr = mmr;

    std::vector<uint8_t> packet = createPacket(MessageType::CONNECT_REQUEST, 0, &request, sizeof(request));

    if (sendto(udpSocket, packet.data(), packet.size(), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) !=
        packet.size())
    {
        perror("sendto");
        close(udpSocket);
        return false;
    }

    connectionSuccess = false;
    std::thread([this]() {
        // Try to receive connection response with timeout
        fd_set readSet;
        struct timeval timeout;

        FD_ZERO(&readSet);
        FD_SET(udpSocket, &readSet);

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // Wait up to 5 seconds for response
        int result = select(udpSocket + 1, &readSet, NULL, NULL, &timeout);

        if (result > 0 && FD_ISSET(udpSocket, &readSet))
        {
            std::vector<uint8_t> buffer(1024);
            sockaddr_in fromAddr;
            socklen_t fromLen = sizeof(fromAddr);

            ssize_t bytesReceived =
                recvfrom(udpSocket, buffer.data(), buffer.size(), 0, (struct sockaddr *)&fromAddr, &fromLen);

            if (bytesReceived > sizeof(NetworkHeader))
            {
                NetworkHeader *header = reinterpret_cast<NetworkHeader *>(buffer.data());

                if (header->type == MessageType::CONNECT_RESPONSE &&
                    bytesReceived >= sizeof(NetworkHeader) + sizeof(ConnectResponse))
                {
                    ConnectResponse *response =
                        reinterpret_cast<ConnectResponse *>(buffer.data() + sizeof(NetworkHeader));

                    if (response->success)
                    {
                        std::cout << "Successfully connected to server" << std::endl;
                        isPlayer1 = response->isPlayer1;
                        connectionSuccess = true;

                        // Save server address for future communication
                        serverAddr = fromAddr;

                        // Store data / notify game
                        if (onMatchFound)
                            onMatchFound(*response);
                    }
                }
            }
        }

        if (!connectionSuccess)
        {
            std::cerr << "Connection to server failed or timed out" << std::endl;
        }

        // Continue listening for game packets
        startListening();
    }).detach();

    // Wait a bit for the connection attempt
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return true; // Just return true here, actual success is determined in the thread
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

    switch (header->type)
    {
    case MessageType::CONNECT_RESPONSE: {
        const ConnectResponse *response =
            reinterpret_cast<const ConnectResponse *>(packet.data() + sizeof(NetworkHeader));

        std::cout << "[MATCHMAKING] Opponent: " << response->opponentName << std::endl;
        std::cout << "Address: " << response->hostAddress << ", UDP: " << response->hostUdpPort
                  << ", TCP: " << response->hostTcpPort << std::endl;
        std::cout << "You are player " << (response->isPlayer1 ? "1" : "2") << std::endl;
        isPlayer1 = response->isPlayer1;

        // Store data / notify game
        std::lock_guard<std::mutex> lock(callbackMutex);
        pendingResponse = *response;
        hasPendingResponse = true;

        break;
    }

    case MessageType::GAME_STATE_UPDATE: {
        if (packet.size() >= sizeof(NetworkHeader) + sizeof(GameState))
        {
            // Parse game state and update the local game state
            GameState state;
            if (state.deserialize(std::vector<uint8_t>(packet.begin() + sizeof(NetworkHeader), packet.end())))
            {
                // Store the latest game state
                latestGameState = state;
                gameStateUpdated = true;
            }
        }
        break;
    }

    case MessageType::SCORE_EVENT: {
        if (packet.size() >= sizeof(NetworkHeader) + sizeof(ScoreEvent))
        {
            const ScoreEvent *event = reinterpret_cast<const ScoreEvent *>(packet.data() + sizeof(NetworkHeader));

            // Notify game to render goal animation
            if (onScoreEvent)
            {
                onScoreEvent(*event);
            }
        }
        break;
    }

    case MessageType::VICTORY_EVENT: {
        if (packet.size() >= sizeof(NetworkHeader) + sizeof(VictoryEvent))
        {
            const VictoryEvent *event = reinterpret_cast<const VictoryEvent *>(packet.data() + sizeof(NetworkHeader));

            // Notify game to show victory screen
            if (onVictoryEvent)
            {
                onVictoryEvent(*event);
            }
        }
        break;
    }

    case MessageType::DISCONNECT_EVENT: {
        // Opponent disconnected
        if (onDisconnectEvent)
        {
            onDisconnectEvent();
        }
        break;
    }

    default:
        // Handle other message types as needed
        break;
    }
}

void NetworkManager::sendPlayerInput(uint8_t inputFlags, uint32_t currentFrame)
{
    PlayerInput input;
    input.playerId = isPlayer1 ? 1 : 2;
    input.flags = inputFlags;
    input.frameNumber = currentFrame;

    if (input.flags == 0)
    {
        return;
    }

    std::vector<uint8_t> packet = createInputPacket(input);

    if (sendto(udpSocket, packet.data(), packet.size(), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) !=
        packet.size())
    {
        std::cerr << "Failed to send input packet" << std::endl;
    }
}

bool NetworkManager::receiveGameState(GameState &state)
{
    if (gameStateUpdated)
    {
        state = latestGameState;
        gameStateUpdated = false;
        return true;
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
    // For direct player-to-player chat (bypassing server as requested)
    std::vector<uint8_t> packet = createChatPacket(username, message);

    // Set up recipient address for direct communication
    sockaddr_in peerAddr;
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(std::stoi(opponentUdpPort));
    inet_pton(AF_INET, serverAddress.c_str(), &peerAddr.sin_addr);

    sendto(udpSocket, packet.data(), packet.size(), 0, (struct sockaddr *)&peerAddr, sizeof(peerAddr));
}

std::vector<uint8_t> NetworkManager::receivePacket()
{
    std::vector<uint8_t> packet(MAX_PACKET_SIZE);
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    int bytesReceived = recvfrom(udpSocket, packet.data(), packet.size(), 0, (struct sockaddr *)&addr, &addrlen);
    if (bytesReceived < 0)
    {
        if (errno == EBADF)
        {
            // Socket was closed, this is expected during shutdown
            return {};
        }
        // Don't print error for EWOULDBLOCK or EAGAIN (non-blocking socket)
        if (errno != EWOULDBLOCK && errno != EAGAIN)
        {
            perror("recvfrom");
        }
        return {};
    }

    packet.resize(bytesReceived);
    return packet;
}

bool NetworkManager::isConnected()
{
    return connectionSuccess;
}

void NetworkManager::processCallbacks()
{
    std::lock_guard<std::mutex> lock(callbackMutex);
    if (hasPendingResponse && onMatchFound)
    {
        onMatchFound(pendingResponse);
        hasPendingResponse = false;
    }
}

} // namespace pong