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
      isPlayer1(true), running(false), ready(false), currentInput(0), udpPort(-1), tcpPort(-1)
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
    if (opponentName == "" && opponentAddress == "" && opponentTcpPort == "0" && opponentUdpPort == "0")
    {
        ready = false;
        running = false;
        return;
    }
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

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
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
        if (gameState.update())
        {
            renderer.renderGoalAnimation();

            int playerScore = gameState.player1.score;
            int AIScore = gameState.player2.score;
            if (playerScore >= gameState.VICTORY_CONDITION || AIScore >= gameState.VICTORY_CONDITION)
            {
                std::string winnerName = (playerScore >= AIScore) ? "P1" : "AI";
                renderer.showVictoryScreen(winnerName, playerScore, AIScore);
                inputHandler.forceQuit();
            }
        }
    }
    else if (gameMode == LOCALMULTIPLAYER)
    {
        updatePlayer1Paddle(currentInput);
        updatePlayer2Paddle(currentInput);
        if (gameState.update())
        {
            renderer.renderGoalAnimation();

            int player1Score = gameState.player1.score;
            int player2Score = gameState.player2.score;
            if (player1Score >= gameState.VICTORY_CONDITION || player2Score >= gameState.VICTORY_CONDITION)
            {
                std::string winnerName = (player1Score >= player2Score) ? "P1" : "P2";
                renderer.showVictoryScreen(winnerName, player1Score, player2Score);
                inputHandler.forceQuit();
            }
        }
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
                gameState.player2.position.y -= 0.15f;
            }
        }
        else if (ballY > centerOfPaddle + 1.0f)
        {
            // Move down
            if (gameState.player2.position.y < GameState::HEIGHT - Paddle::HEIGHT - 1)
            {
                gameState.player2.position.y += 0.15f;
            }
        }
    }
}

void Game::toggleChat()
{
    if (chatActive)
        return;

    chatActive = true;
    inputHandler.startChatMode();

    std::thread([this]() {
        std::string message = inputHandler.getChatInput();
        if (!message.empty())
        {
            networkManager.sendChatMessage(message);
            ChatMessageData cm;
            std::string name = "You";
            strncpy(cm.content, message.data(), message.size());
            cm.contentLength = message.size();
            strncpy(cm.sender, name.data(), name.size());
            auto now = std::chrono::system_clock::now();
            cm.timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

            addChatMessage(cm);
            renderer.renderChatMessages(getChatMessages());
        }
        chatActive = false;
        inputHandler.stopChatMode();
    }).detach();
}

void Game::addChatMessage(const ChatMessageData message)
{
    std::cout << "NEW CHAT MESSAGE: " << message.sender << ":" << message.content << std::endl;

    // Add to the chat messages vector
    chatMessages.push_back(message);

    // Limit the number of messages to display (keep the most recent ones)
    const size_t MAX_MESSAGES = 5;
    if (chatMessages.size() > MAX_MESSAGES)
    {
        chatMessages.erase(chatMessages.begin(), chatMessages.begin() + (chatMessages.size() - MAX_MESSAGES));
    }
}

const std::vector<ChatMessageData> &Game::getChatMessages() const
{
    return chatMessages;
}
} // namespace pong