#pragma once
#include "../common/game_state.h"
#include "../common/network.h"

#include "matchmaker.h"
#include "network.h"
#include <mutex>
#include <vector>

namespace pong
{

class Matchmaker;     // Forward declaration
class NetworkManager; // Forward declaration

class ServerGame
{
  public:
    ServerGame() : running(true), active(false), matchmaker(nullptr), networkManager(nullptr)
    {
        gameState.reset(rand() % 2 == 0);
    }

    void startGame()
    {
        active = true;
        gameState.reset(rand() % 2 == 0);
    }
    void stopGame()
    {
        active = false;
    }
    bool isActive() const
    {
        return active;
    }
    void setMatchmaker(Matchmaker *matchmaker)
    {
        this->matchmaker = matchmaker;
    }
    void setNetworkManager(NetworkManager *networkManager)
    {
        this->networkManager = networkManager;
    }

    void update();
    void addPlayerInput(uint8_t playerId, uint8_t inputFlags);
    const GameState &getGameState() const;

  private:
    void processPlayerInputs();
    void handleGoalScored();
    void handleVictory();
    GameState gameState;
    std::vector<PlayerInput> pendingInputs;
    std::mutex inputMutex;
    bool running;
    bool active;

    Matchmaker *matchmaker;
    NetworkManager *networkManager;
};

} // namespace pong