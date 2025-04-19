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
    ONLINE,
    LOCALMULTIPLAYER
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
    void updatePlayerPaddle(uint8_t input);
    void updatePlayer1Paddle(uint8_t input);
    void updatePlayer2Paddle(uint8_t input);
    void updateAI();

    InputHandler &inputHandler;
    Renderer &renderer;
    NetworkManager &networkManager;
    GameState gameState;
    GameMode gameMode;
    bool isPlayer1;
    uint8_t currentInput;
    std::string opponentName;
    std::string opponentAddress;
    std::string opponentUdpPort;
    std::string opponentTcpPort;

    float playerX, opponentX;
};

} // namespace pong