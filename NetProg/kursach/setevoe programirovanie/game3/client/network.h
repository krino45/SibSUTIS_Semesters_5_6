// client/network.h

#pragma once

#include "../common/network.h"
#include <arpa/inet.h>
#include <atomic>
#include <condition_variable>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace pong
{

class NetworkManager
{
  public:
    NetworkManager();
    ~NetworkManager();

    bool connectToServer(const std::string &serverAddress, uint16_t udpPort, uint16_t tcpPort,
                         const std::string &username, uint32_t mmr);
    void sendPlayerInput(uint8_t inputFlags, uint32_t currentFrame);
    bool receiveGameState(GameState &state);
    void startListening();
    void handlePacket(const std::vector<uint8_t> &packet);
    bool isConnected();
    void processCallbacks();

    bool startChatServer(uint16_t port);
    bool connectToChat(const std::string &address, uint16_t port);
    void sendChatMessage(const std::string &message);
    std::vector<ChatMessageData> getChatMessages();
    void stopChat();

    std::function<void(const ConnectResponse &)> onMatchFound;
    std::function<void()> onDisconnectEvent;
    std::function<void(const VictoryEvent &)> onVictoryEvent;
    std::function<void(const ScoreEvent &)> onScoreEvent;
    std::function<void(const ChatMessageData &)> onChatMessage;

  private:
    ConnectResponse pendingResponse;
    sockaddr_in serverAddr;
    int udpSocket;
    int tcpSocket;
    std::thread chatThread;
    std::atomic<bool> chatRunning{false};
    std::vector<ChatMessageData> chatMessages;
    std::mutex chatMutex;
    int chatClientSocket;

    std::string serverAddress;
    uint16_t udpPort;
    uint16_t tcpPort;
    std::string username;
    uint32_t mmr;
    std::mutex mutex;
    std::mutex callbackMutex;
    std::mutex connectionMutex;
    std::mutex gameStateMutex;
    std::condition_variable connectionCV;
    bool connectionSuccess = false;
    bool isPlayer1;
    bool hasPendingResponse;
    bool gameStateUpdated;
    GameState latestGameState;
    std::thread listenThread;
    bool running;

    // void sendPacket(const std::vector<uint8_t> &packet, std::string opponentUdpPort);
    std::vector<uint8_t> receivePacket();
};

} // namespace pong
