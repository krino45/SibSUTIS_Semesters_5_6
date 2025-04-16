// client/game.h
#pragma once

#include "../common/game_state.h"
#include "input.h"
#include "network.h"
#include "render.h"

namespace pong
{

enum GameMode
{
    LOCAL,
    ONLINE
};

class Game
{
  public:
    Game(InputHandler &inputHandler, Renderer &renderer, NetworkManager &networkManager);
    ~Game();

    void setGameMode(GameMode mode);
    void setIsPlayer1(bool isP1);
    void start();
    void update();
    const GameState &getGameState() const;
    void setOpponentInfo(const ConnectResponse &response);
    bool ready;
    bool running;

  private:
    void handleInput();
    void updatePlayer1Paddle(uint8_t input);
    void updatePlayer2Paddle(uint8_t input);
    void updateAI();
    void updateGameState();
    void resetBall();

    InputHandler &inputHandler;
    Renderer &renderer;
    NetworkManager &networkManager;
    GameState gameState;
    GameMode gameMode;
    bool isPlayer1;
    std::string opponentName;
    std::string opponentAddress;
    std::string opponentUdpPort;
    std::string opponentTcpPort;

    float playerX, opponentX;
};

} // namespace pong