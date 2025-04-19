#pragma once
#include "../common/game_state.h"
#include "../common/network.h"
#include <mutex>
#include <vector>

namespace pong
{

class ServerGame
{
  public:
    ServerGame() : running(true), active(false)
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
    void update();
    void addPlayerInput(uint8_t playerId, uint8_t inputFlags);
    const GameState &getGameState() const;

  private:
    void processPlayerInputs();

    GameState gameState;
    std::vector<PlayerInput> pendingInputs;
    std::mutex inputMutex;
    bool running;
    bool active;
};

} // namespace pong