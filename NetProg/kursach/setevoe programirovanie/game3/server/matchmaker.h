// server/matchmaker.h
#pragma once
#include "../common/network.h"
#include "game_instance.h"
#include "game_manager.h"
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>

namespace pong
{

// Forward declaration
class NetworkManager;
class GameInstance;
class GameManager;

struct PlayerInfo
{
    std::string username;
    std::string clientId;
    std::string address;
    uint16_t udpPort;
    uint16_t tcpPort; // For direct player-to-player chat
    uint32_t mmr;
};

class Matchmaker
{
  public:
    Matchmaker();
    ~Matchmaker();

    // Set the network manager reference
    void setNetworkManager(NetworkManager *manager)
    {
        this->networkManager = manager;
    }

    void setGameManager(GameManager *manager)
    {
        this->gameManager = manager;
    }

    // Player management
    uint8_t registerPlayer(const PlayerInfo &player);

    // Matchmaking logic
    void process();

    // Match notification
    void notifyPlayersAboutMatch(const PlayerInfo &player1, const PlayerInfo &player2);

    std::string getPlayer1Name();
    std::string getPlayer2Name();

    void handlePlayerDisconnect(const std::string &clientId);

  private:
    // Find a match among waiting players
    bool findMatch(PlayerInfo &player1, PlayerInfo &player2);

    // Create match notification packet
    std::vector<uint8_t> createMatchNotificationPacket(const PlayerInfo &player, const PlayerInfo &opponent);

    // Queue and matching data
    std::mutex queueMutex;
    std::mutex playersMutex;
    std::queue<PlayerInfo> waitingPlayers;

    std::unordered_map<std::string, PlayerInfo> activePlayersByUsername;
    std::unordered_map<std::string, PlayerInfo> activePlayersByClientId;

    std::string currentPlayer1;
    std::string currentPlayer2;

    // Reference to the network manager for sending packets
    NetworkManager *networkManager;
    GameManager *gameManager;
};

} // namespace pong
