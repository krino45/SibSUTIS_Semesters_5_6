// client/network.cpp

#include "network.h"

namespace pong
{

NetworkManager::NetworkManager()
    : udpSocket(-1), tcpSocket(-1), hasPendingResponse(false), pendingResponse({}), chatClientSocket(-1)
{
}

NetworkManager::~NetworkManager()
{
    running = false;

    // Force wakeup the network thread if it's blocked in recvfrom
    if (udpSocket != -1 && udpSocket != 0)
    {
        shutdown(udpSocket, SHUT_RDWR);
        if (serverAddr.sin_port != 0)
        {
            sendto(udpSocket, "", 0, 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        }
        close(udpSocket);
        udpSocket = -1;
    }

    if (tcpSocket != -1 && udpSocket != 0)
    {
        shutdown(tcpSocket, SHUT_RDWR);
        close(tcpSocket);
        tcpSocket = -1;
    }

    if (listenThread.joinable())
    {
        listenThread.join();
    }

    if (chatThread.joinable())
    {
        chatThread.join();
    }
}

bool NetworkManager::connectToServer(const std::string &serverAddress, uint16_t udpPort, uint16_t tcpPort,
                                     const std::string &username)
{
    this->serverAddress = serverAddress;
    this->udpPort = udpPort;
    this->username = username;

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
    request.mmr = 69;          // unneeded

    std::vector<uint8_t> packet = createPacket(MessageType::CONNECT_REQUEST, 0, &request, sizeof(request));

    if (sendto(udpSocket, packet.data(), packet.size(), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) !=
        packet.size())
    {
        perror("sendto");
        close(udpSocket);
        return false;
    }

    connectionSuccess = false;
    bool connectionDeclined = false;
    running = true;
    std::thread connectionThread([&]() {
        fd_set readSet;
        struct timeval timeout;

        FD_ZERO(&readSet);
        FD_SET(udpSocket, &readSet);

        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

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
                        std::lock_guard<std::mutex> lock(connectionMutex);
                        isPlayer1 = response->isPlayer1;
                        serverAddr = fromAddr;
                        connectionSuccess = true;

                        std::cout << "Successfully connected to server" << std::endl;
                        if (onMatchFound)
                            onMatchFound(*response);
                    }
                    else
                    {
                        connectionDeclined = true;
                    }
                    connectionCV.notify_one();
                }
            }
        }

        startListening(); // Continue game packet listening
    });
    connectionThread.detach();

    {
        std::unique_lock<std::mutex> lock(connectionMutex);
        connectionCV.wait_for(lock, std::chrono::seconds(5), [this]() { return connectionSuccess; });
    }

    if (connectionDeclined)
    {
        std::cerr << "Connection declined by server (are you already logged in?)" << std::endl;
        return false;
    }

    if (!connectionSuccess)
    {
        std::cerr << "Connection to server failed or timed out" << std::endl;
        return false;
    }

    return true;
}

void NetworkManager::startListening()
{
    listenThread = std::thread([this]() {
        while (running)
        {
            std::vector<uint8_t> packet = receivePacket();
            if (!packet.empty())
            {
                handlePacket(packet);
            }
        }
    });
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

        std::cout << "[MATCHMAKING] Opponent: " << response->opponentName << "(" << response->mmr << ")" << std::endl;
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
                std::lock_guard<std::mutex> lock(gameStateMutex);
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
        else
        {
            std::cerr << "want: >=" << sizeof(NetworkHeader) + sizeof(ScoreEvent) << ", got: " << packet.size()
                      << std::endl;
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
        else
        {
            std::cerr << "want: >=" << sizeof(NetworkHeader) + sizeof(VictoryEvent) << ", got: " << packet.size()
                      << std::endl;
        }
        break;
    }

    case MessageType::DISCONNECT_EVENT: {
        if (onDisconnectEvent)
        {
            onDisconnectEvent();
        }
        break;
    }

    default:
        std::cerr << "Unknown message type: " << static_cast<int>(header->type) << "\n";
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
    std::lock_guard<std::mutex> lock(gameStateMutex);
    state = latestGameState;
    bool updated = gameStateUpdated;
    gameStateUpdated = false;
    return true;
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

    // Make a local copy of the response to ensure thread safety
    ConnectResponse response;
    bool shouldProcess = false;

    if (hasPendingResponse)
    {
        // Copy all fields explicitly to avoid padding issues
        response.success = pendingResponse.success;
        response.isPlayer1 = pendingResponse.isPlayer1;
        response.hostUdpPort = pendingResponse.hostUdpPort;
        response.hostTcpPort = pendingResponse.hostTcpPort;
        strncpy(response.hostAddress, pendingResponse.hostAddress, sizeof(response.hostAddress));
        strncpy(response.opponentName, pendingResponse.opponentName, sizeof(response.opponentName));
        response.mmr = pendingResponse.mmr;

        shouldProcess = true;
        hasPendingResponse = false;
    }

    if (shouldProcess && onMatchFound)
    {
        onMatchFound(response);
    }
}

bool NetworkManager::startChatServer(uint16_t port)
{
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1)
    {
        perror("chat socket");
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(tcpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)))
    {
        perror("chat bind");
        close(tcpSocket);
        return false;
    }

    if (listen(tcpSocket, 10))
    {
        perror("chat listen");
        close(tcpSocket);
        return false;
    }

    chatRunning = true;
    chatThread = std::thread([this]() {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(tcpSocket, (struct sockaddr *)&clientAddr, &clientLen);
        chatClientSocket = clientSocket;

        if (clientSocket < 0)
        {
            perror("chat accept");
            return;
        }

        std::vector<uint8_t> buffer(sizeof(NetworkHeader) + sizeof(ChatMessageData));
        while (chatRunning)
        {
            ssize_t bytes = recv(clientSocket, buffer.data(), buffer.size(), 0);
            if (bytes <= 0)
            {
                if (bytes == 0)
                {
                    // std::cerr << "Connection closed by peer." << std::endl;
                }
                else
                {
                    std::cerr << "recv error: " << strerror(errno) << std::endl;
                }
                break;
            }

            std::lock_guard<std::mutex> lock(chatMutex);

            const NetworkHeader *header = reinterpret_cast<const NetworkHeader *>(buffer.data());
            ChatMessageData *message = reinterpret_cast<ChatMessageData *>(buffer.data() + sizeof(NetworkHeader));

            chatMessages.push_back(*message);
            if (onChatMessage)
            {
                onChatMessage(*message);
            }
        }
        close(clientSocket);
    });

    return true;
}

bool NetworkManager::connectToChat(const std::string &address, uint16_t port)
{
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == -1)
    {
        perror("chat socket");
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, address.c_str(), &serverAddr.sin_addr);

    if (connect(tcpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)))
    {
        perror("chat connect");
        close(tcpSocket);
        return false;
    }

    chatRunning = true;
    chatThread = std::thread([this]() {
        std::vector<uint8_t> buffer(sizeof(NetworkHeader) + sizeof(ChatMessageData));
        while (chatRunning)
        {
            ssize_t bytes = recv(tcpSocket, buffer.data(), buffer.size(), 0);
            if (bytes <= 0)
            {
                if (bytes == 0)
                {
                    // std::cerr << "Connection closed by peer." << std::endl;
                }
                else
                {
                    std::cerr << "recv error: " << strerror(errno) << std::endl;
                }
                break;
            }
            std::lock_guard<std::mutex> lock(chatMutex);

            const NetworkHeader *header = reinterpret_cast<const NetworkHeader *>(buffer.data());
            const ChatMessageData *message =
                reinterpret_cast<const ChatMessageData *>(buffer.data() + sizeof(NetworkHeader));
            chatMessages.push_back(*message);
            if (onChatMessage)
            {
                onChatMessage(*message);
            }
        }
    });

    return true;
}

void NetworkManager::sendChatMessage(const std::string &message_text)
{
    int socket = chatClientSocket != -1 ? chatClientSocket : tcpSocket;
    if (socket != -1)
    {
        std::vector<uint8_t> packet = createChatPacket(username, message_text);
        std::cout << "Sending to tcpSocket on " << socket << "\n" << std::flush;
        ssize_t bytesSent = send(socket, packet.data(), packet.size(), MSG_NOSIGNAL);
        if (bytesSent <= 0)
        {
            if (bytesSent == 0)
            {
                // std::cerr << "Connection closed by peer." << std::endl;
            }
            else
            {
                std::cerr << "send error: " << strerror(errno) << std::endl;
            }
        }
    }
    else
    {
        std::cerr << "TCP socket is not open!" << std::endl;
        return;
    }
}

std::vector<ChatMessageData> NetworkManager::getChatMessages()
{
    std::lock_guard<std::mutex> lock(chatMutex);
    return chatMessages;
}

void NetworkManager::stopChat()
{
    chatRunning = false;
    if (tcpSocket != -1)
    {
        shutdown(tcpSocket, SHUT_RDWR);
        close(tcpSocket);
        tcpSocket = -1;
    }
    if (chatThread.joinable())
    {
        chatThread.join();
    }
}

} // namespace pong