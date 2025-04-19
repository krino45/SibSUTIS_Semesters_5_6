// server/matchmaker.h

#pragma once

#include "../common/network.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace pong
{

struct PlayerInfo
{
    std::string username;
    uint16_t udpPort;
    uint16_t tcpPort;
    uint32_t mmr;
    std::string address;
};

struct MatchNotification
{
    char opponentUsername[256];
    uint16_t opponentUdpPort;
    uint16_t opponentTcpPort;
};

class Matchmaker
{
  public:
    Matchmaker();
    ~Matchmaker();

    uint8_t addPlayer(const PlayerInfo &player);
    bool findMatch(PlayerInfo &player1, PlayerInfo &player2);
    void process();

    void notifyPlayerAboutMatch(const PlayerInfo &player, const PlayerInfo &opponent);

    std::vector<uint8_t> createMatchNotificationPacket(const PlayerInfo &player, const PlayerInfo &opponent);

    void sendPacketToPlayer(const PlayerInfo &player, const std::vector<uint8_t> &packet);

  private:
    std::mutex mutex;
    std::condition_variable cv;
    std::queue<PlayerInfo> waitingPlayers;
    std::unordered_map<std::string, PlayerInfo> matchedPlayers;
};

} // namespace pong
