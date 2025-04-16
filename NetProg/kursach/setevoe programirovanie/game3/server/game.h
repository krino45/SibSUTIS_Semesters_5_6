#pragma once
#include "../common/game_state.h"
#include "../common/network.h"
#include <mutex>
#include <vector>

namespace pong
{

struct PlayerInput
{
    uint8_t playerId;
    uint8_t flags;
};

class ServerGame
{
  public:
    ServerGame();

    void update();
    void addPlayerInput(uint8_t playerId, uint8_t inputFlags);
    const GameState &getGameState() const;

  private:
    void processPlayerInputs();

    GameState gameState;
    std::vector<PlayerInput> pendingInputs;
    std::mutex inputMutex;
    bool running;
};

} // namespace pong