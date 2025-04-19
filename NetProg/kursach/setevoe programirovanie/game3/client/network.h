// client/network.h

#pragma once

#include "../common/network.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
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
    void receiveChatMessage(std::string &message);
    void startListening();
    void handlePacket(const std::vector<uint8_t> &packet);
    bool isConnected();
    std::function<void(const ConnectResponse &)> onMatchFound;

  private:
    int udpSocket;
    int tcpSocket;
    std::string serverAddress;
    uint16_t udpPort;
    uint16_t tcpPort;
    std::string username;
    uint32_t mmr;
    std::mutex mutex;
    bool isPlayer1;

    void sendPacket(const std::vector<uint8_t> &packet, std::string opponentUdpPort);
    std::vector<uint8_t> receivePacket();
};

} // namespace pong
