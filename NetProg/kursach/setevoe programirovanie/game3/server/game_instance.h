// server / game_instance.h

#pragma once
#include "../common/game_state.h"
#include "../common/network.h"
#include "matchmaker.h"
#include "network.h"
#include <mutex>
#include <vector>

namespace pong
{

class NetworkManager;
class Matchmaker;

class GameInstance
{
  public:
    GameInstance(uint32_t id, const std::string &player1, const std::string &player2);

    void update();
    void addPlayerInput(uint8_t playerId, uint8_t inputFlags);
    const GameState &getGameState() const;
    bool isActive() const;
    void stopGame();
    void startGame(); // New method
    uint32_t getId() const;
    bool hasPlayer(const std::string &clientId) const;
    std::vector<std::string> getAllPlayers() const;
    void broadcastState(NetworkManager *networkManager);
    void setNetworkManager(NetworkManager *networkManager)
    {
        networkManager_ = networkManager;
    }
    void setMatchmaker(Matchmaker *matchmaker)
    {
        matchmaker_ = matchmaker;
    }

  private:
    void processPlayerInputs();
    void handleGoalScored();
    void handleVictory();

    uint32_t id_;
    GameState gameState_;
    std::vector<PlayerInput> pendingInputs_;
    std::mutex inputMutex_;
    bool active_;
    std::string player1Id_;
    std::string player2Id_;
    NetworkManager *networkManager_;
    Matchmaker *matchmaker_;
    uint32_t frameCounter_;
};

} // namespace pong