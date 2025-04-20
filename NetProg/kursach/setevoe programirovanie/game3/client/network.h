// client/network.h

#pragma once

#include "../common/network.h"
#include <arpa/inet.h>
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
    std::vector<uint8_t> createInputPacket(const PlayerInput &input);
    void sendChatMessage(const std::string &message, std::string opponentUdpPort);
    // void receiveChatMessage(std::string &message);
    void startListening();
    void handlePacket(const std::vector<uint8_t> &packet);
    bool isConnected();
    void processCallbacks();
    std::function<void(const ConnectResponse &)> onMatchFound;
    std::function<void()> onDisconnectEvent;
    std::function<void(const VictoryEvent &)> onVictoryEvent;
    std::function<void(const ScoreEvent &)> onScoreEvent;

  private:
    ConnectResponse pendingResponse;
    sockaddr_in serverAddr;
    int udpSocket;
    int tcpSocket;
    std::string serverAddress;
    bool connectionSuccess;
    uint16_t udpPort;
    uint16_t tcpPort;
    std::string username;
    uint32_t mmr;
    std::mutex mutex;
    std::mutex callbackMutex;
    bool isPlayer1;
    bool hasPendingResponse;
    bool gameStateUpdated;
    GameState latestGameState;

    // void sendPacket(const std::vector<uint8_t> &packet, std::string opponentUdpPort);
    std::vector<uint8_t> receivePacket();
};

} // namespace pong
