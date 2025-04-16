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

void Game::update() // MAIN method
{
    handleInput();

    if (gameMode == GameMode::ONLINE)
    {
        if (isPlayer1)
        {
            updateGameState();
            networkManager.sendGameState(gameState, opponentUdpPort);
        }
        else
        {
            std::cout << "mememeim player2 \n";
            GameState receivedState;
            networkManager.receiveGameState(receivedState);

            receivedState.player2.position = gameState.player2.position;
            gameState = receivedState;
        }
    }
    else
    {
        updateAI();
        updateGameState();
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
        return;
    }

    if (gameMode == GameMode::ONLINE)
    {
        if (isPlayer1)
        {
            updatePlayer1Paddle(input);
        }
        else
        {
            updatePlayer2Paddle(input);
        }
    }
    else
    {
        updatePlayer1Paddle(input);
    }
}

void Game::updatePlayer1Paddle(uint8_t input)
{
    if (input & InputFlags::UP)
    {
        if (gameState.player1.position.y > 0)
        {
            gameState.player1.position.y -= 1;
        }
    }
    if (input & InputFlags::DOWN)
    {
        if (gameState.player1.position.y < GameState::HEIGHT - Paddle::HEIGHT)
        {
            gameState.player1.position.y += 1;
        }
    }
}

void Game::updatePlayer2Paddle(uint8_t input)
{
    if (input & InputFlags::UP)
    {
        if (gameState.player2.position.y > 0)
        {
            gameState.player2.position.y -= 1;
        }
    }
    if (input & InputFlags::DOWN)
    {
        if (gameState.player2.position.y < GameState::HEIGHT - Paddle::HEIGHT)
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
            if (gameState.player2.position.y > 0)
            {
                gameState.player2.position.y -= 0.5f;
            }
        }
        else if (ballY > centerOfPaddle + 1.0f)
        {
            // Move down
            if (gameState.player2.position.y < GameState::HEIGHT - Paddle::HEIGHT)
            {
                gameState.player2.position.y += 0.5f;
            }
        }
    }
}

void Game::updateGameState()
{
    // Update ball position
    gameState.ball.position = gameState.ball.position + gameState.ball.velocity;

    // Check for collision with top and bottom walls
    if (gameState.ball.position.y <= 0 || gameState.ball.position.y >= GameState::HEIGHT - 1)
    {
        gameState.ball.velocity.y = -gameState.ball.velocity.y;
        // Clamp position to prevent getting stuck in the wall
        if (gameState.ball.position.y <= 0)
        {
            gameState.ball.position.y = 0;
        }
        else
        {
            gameState.ball.position.y = GameState::HEIGHT - 1;
        }
    }

    // Check for collision with paddles
    if (gameState.ball.position.x <= gameState.player1.position.x + Paddle::WIDTH &&
        gameState.ball.position.x >= gameState.player1.position.x &&
        gameState.ball.position.y >= gameState.player1.position.y &&
        gameState.ball.position.y <= gameState.player1.position.y + Paddle::HEIGHT)
    {

        // Calculate reflection angle based on where the ball hit the paddle
        float relativeIntersectY = (gameState.player1.position.y + (Paddle::HEIGHT / 2)) - gameState.ball.position.y;
        float normalizedRelativeIntersectionY = (relativeIntersectY / (Paddle::HEIGHT / 2));
        float bounceAngle = normalizedRelativeIntersectionY * (3.14159f / 4); // Max 45 degrees

        gameState.ball.velocity.x = std::abs(gameState.ball.velocity.x); // Force direction away from paddle
        gameState.ball.velocity.y = -std::sin(bounceAngle) * gameState.ball.velocity.x;

        // Add a small speed increase on each hit
        const float speedIncrease = 0.1f;
        float currentSpeed = std::sqrt(gameState.ball.velocity.x * gameState.ball.velocity.x +
                                       gameState.ball.velocity.y * gameState.ball.velocity.y);
        float ratio = (currentSpeed + speedIncrease) / currentSpeed;

        gameState.ball.velocity.x *= ratio;
        gameState.ball.velocity.y *= ratio;
    }

    if (gameState.ball.position.x >= gameState.player2.position.x - Ball::RADIUS &&
        gameState.ball.position.x <= gameState.player2.position.x &&
        gameState.ball.position.y >= gameState.player2.position.y &&
        gameState.ball.position.y <= gameState.player2.position.y + Paddle::HEIGHT)
    {

        // Calculate reflection angle based on where the ball hit the paddle
        float relativeIntersectY = (gameState.player2.position.y + (Paddle::HEIGHT / 2)) - gameState.ball.position.y;
        float normalizedRelativeIntersectionY = (relativeIntersectY / (Paddle::HEIGHT / 2));
        float bounceAngle = normalizedRelativeIntersectionY * (3.14159f / 4); // Max 45 degrees

        gameState.ball.velocity.x = -std::abs(gameState.ball.velocity.x); // Force direction away from paddle
        gameState.ball.velocity.y = -std::sin(bounceAngle) * std::abs(gameState.ball.velocity.x);

        // Add a small speed increase on each hit
        const float speedIncrease = 0.1f;
        float currentSpeed = std::sqrt(gameState.ball.velocity.x * gameState.ball.velocity.x +
                                       gameState.ball.velocity.y * gameState.ball.velocity.y);
        float ratio = (currentSpeed + speedIncrease) / currentSpeed;

        gameState.ball.velocity.x *= ratio;
        gameState.ball.velocity.y *= ratio;
    }

    // Check for scoring
    if (gameState.ball.position.x <= 0)
    {
        gameState.player2.score++;
        resetBall();
    }

    if (gameState.ball.position.x >= GameState::WIDTH - 1)
    {
        gameState.player1.score++;
        resetBall();
    }

    // Update frame
    gameState.frame++;
}

void Game::resetBall()
{
    // Reset ball to center with random direction
    gameState.ball.position.x = GameState::WIDTH / 2.0f;
    gameState.ball.position.y = GameState::HEIGHT / 2.0f;

    // Random angle between -PI/4 and PI/4
    float angle = ((std::rand() % 100) / 100.0f - 0.5f) * (3.14159f / 2);

    // Direction alternates based on who scored
    float direction = (gameState.frame % 2 == 0) ? 1.0f : -1.0f;

    const float initialSpeed = 0.5f;
    gameState.ball.velocity.x = direction * initialSpeed * std::cos(angle);
    gameState.ball.velocity.y = initialSpeed * std::sin(angle);
}

} // namespace pong