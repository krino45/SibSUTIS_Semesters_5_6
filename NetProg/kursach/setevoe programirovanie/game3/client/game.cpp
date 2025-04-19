// client/game.cpp
#include "game.h"
#include "../common/utils.h"
#include <chrono>
#include <iostream>
#include <thread>

namespace pong
{

Game::Game(InputHandler &inputHandler, Renderer &renderer, NetworkManager &networkManager)
    : inputHandler(inputHandler), renderer(renderer), networkManager(networkManager), gameMode(GameMode::LOCAL),
      isPlayer1(true), running(true)
{
    gameState.reset(rand() % 2 == 0);
}

Game::~Game()
{
}

void Game::setGameMode(GameMode mode)
{
    gameMode = mode;
}

void Game::setOpponentInfo(const ConnectResponse &response)
{
    opponentName = response.opponentName;
    opponentAddress = response.hostAddress;
    opponentUdpPort = std::to_string(response.hostUdpPort);
    opponentTcpPort = std::to_string(response.hostTcpPort);
    ready = true;

    std::cout << "!!!!! Opponent info set: " << opponentName << "@" << opponentAddress << ":udp" << opponentUdpPort
              << ":tcp" << opponentTcpPort << std::endl;
}

void Game::setIsPlayer1(bool isP1)
{
    isPlayer1 = isP1;
}

void Game::start()
{
    inputHandler.enableRawMode();
    running = true;
    gameState.reset(rand() % 2 == 0);

    auto lastFrameTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds frameTime(16); // ~60 FPS

    while (running)
    {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFrameTime);

        if (elapsedTime >= frameTime)
        {
            update();
            renderer.renderGameState(gameState);
            lastFrameTime = currentTime;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Game::update()
{
    handleInput();

    if (gameMode == GameMode::ONLINE)
    {
        networkManager.sendPlayerInput(currentInput, gameState.frame);

        // Receive and apply game state from server
        GameState receivedState;
        if (networkManager.receiveGameState(receivedState))
        {
            gameState = receivedState;
        }
    }
    else if (gameMode == LOCAL)
    {
        updatePlayerPaddle(currentInput);
        updateAI();
        gameState.update();
    }
    else if (gameMode == LOCALMULTIPLAYER)
    {
        updatePlayer1Paddle(currentInput);
        updatePlayer2Paddle(currentInput);
        gameState.update();
    }
}

const GameState &Game::getGameState() const
{
    return gameState;
}

void Game::handleInput()
{
    uint8_t input = inputHandler.poll();

    if (input & InputFlags::QUIT)
    {
        running = false;
    }

    currentInput = input;
}

void Game::updatePlayerPaddle(uint8_t input)
{
    if (input & InputFlags::UP || input & InputFlags::ARROW_UP)
    {
        if (gameState.player1.position.y > 1)
        {
            gameState.player1.position.y -= 1;
        }
    }
    if (input & InputFlags::DOWN || input & InputFlags::ARROW_DOWN)
    {
        if (gameState.player1.position.y < GameState::HEIGHT - Paddle::HEIGHT - 1)
        {
            gameState.player1.position.y += 1;
        }
    }
}

void Game::updatePlayer1Paddle(uint8_t input)
{
    if (input & InputFlags::UP)
    {
        if (gameState.player1.position.y > 1)
        {
            gameState.player1.position.y -= 1;
        }
    }
    if (input & InputFlags::DOWN)
    {
        if (gameState.player1.position.y < GameState::HEIGHT - Paddle::HEIGHT - 1)
        {
            gameState.player1.position.y += 1;
        }
    }
}

void Game::updatePlayer2Paddle(uint8_t input)
{
    if (input & InputFlags::ARROW_UP)
    {
        if (gameState.player2.position.y > 1)
        {
            gameState.player2.position.y -= 1;
        }
    }
    if (input & InputFlags::ARROW_DOWN)
    {
        if (gameState.player2.position.y < GameState::HEIGHT - Paddle::HEIGHT - 1)
        {
            gameState.player2.position.y += 1;
        }
    }
}

void Game::updateAI()
{
    // Simple AI: follow the ball
    const float centerOfPaddle = gameState.player2.position.y + Paddle::HEIGHT / 2.0f;
    const float ballY = gameState.ball.position.y;

    // Only move if the ball is coming towards the AI
    if (gameState.ball.velocity.x > 0)
    {
        if (ballY < centerOfPaddle - 1.0f)
        {
            // Move up
            if (gameState.player2.position.y > 1)
            {
                gameState.player2.position.y -= 0.2f;
            }
        }
        else if (ballY > centerOfPaddle + 1.0f)
        {
            // Move down
            if (gameState.player2.position.y < GameState::HEIGHT - Paddle::HEIGHT - 1)
            {
                gameState.player2.position.y += 0.2f;
            }
        }
    }
}
} // namespace pong