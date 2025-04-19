// server / game.cpp
#include "game.h"
#include <algorithm>
#include <iostream>

namespace pong
{

void ServerGame::update()
{
    // Update ball position and handle collisions
    gameState.update();

    // Process all pending player inputs
    processPlayerInputs();
}

void ServerGame::processPlayerInputs()
{
    std::lock_guard<std::mutex> lock(inputMutex);
    // std::cout << "PRocessing pending inputs!\n";
    for (const auto &input : pendingInputs)
    {
        if (input.playerId == 1)
        {
            if (input.flags & InputFlags::UP || input.flags & InputFlags::ARROW_UP)
            {
                if (gameState.player1.position.y > 1)
                {
                    gameState.player1.position.y -= 1;
                    std::cout << "Moving P1 up to " << gameState.player1.position.y << std::endl;
                }
            }
            if (input.flags & InputFlags::DOWN || input.flags & InputFlags::ARROW_DOWN)
            {
                if (gameState.player1.position.y < GameState::HEIGHT - Paddle::HEIGHT - 1)
                {
                    gameState.player1.position.y += 1;
                    std::cout << "Moving P1 down to " << gameState.player1.position.y << std::endl;
                }
            }
        }
        else if (input.playerId == 2)
        {
            if (input.flags & InputFlags::UP || input.flags & InputFlags::ARROW_UP)
            {
                if (gameState.player2.position.y > 1)
                {
                    gameState.player2.position.y -= 1;
                    std::cout << "Moving P2 up to " << gameState.player1.position.y << std::endl;
                }
            }
            if (input.flags & InputFlags::DOWN || input.flags & InputFlags::ARROW_DOWN)
            {
                if (gameState.player2.position.y < GameState::HEIGHT - Paddle::HEIGHT - 1)
                {
                    gameState.player2.position.y += 1;
                    std::cout << "Moving P2 down to " << gameState.player1.position.y << std::endl;
                }
            }
        }
    }
    pendingInputs.clear();
}

void ServerGame::addPlayerInput(uint8_t playerId, uint8_t inputFlags)
{
    std::lock_guard<std::mutex> lock(inputMutex);
    pendingInputs.push_back({playerId, inputFlags});

    std::cout << "Received input - Player: " << (int)playerId << " Flags: " << (int)inputFlags << std::endl;
}

const GameState &ServerGame::getGameState() const
{
    return gameState;
}

} // namespace pong