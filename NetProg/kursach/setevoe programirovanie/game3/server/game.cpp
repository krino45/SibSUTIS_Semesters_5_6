// server / game.cpp
#include "game.h"
#include <algorithm>
#include <bits/this_thread_sleep.h>
#include <iostream>

namespace pong
{

void ServerGame::update()
{
    // Update ball position and handle collisions
    if (gameState.update())
    {
        // A goal was scored
        handleGoalScored();
    }
    // Process all pending player inputs
    processPlayerInputs();
}

void ServerGame::handleGoalScored()
{
    // Create score event packet
    ScoreEvent scoreEvent;
    scoreEvent.scoringPlayer = (gameState.lastScoringPlayerIsPlayer1) ? 1 : 2;
    scoreEvent.player1Score = gameState.player1.score;
    scoreEvent.player2Score = gameState.player2.score;

    std::vector<uint8_t> packet =
        createPacket(MessageType::SCORE_EVENT, gameState.frame, &scoreEvent, sizeof(scoreEvent));

    if (networkManager)
    {
        networkManager->broadcastToAllClients(packet);
    }

    if (scoreEvent.player1Score >= GameState::VICTORY_CONDITION ||
        scoreEvent.player2Score >= GameState::VICTORY_CONDITION)
    {
        handleVictory();
    }
}

void ServerGame::handleVictory()
{
    VictoryEvent victoryEvent;
    victoryEvent.winningPlayer = (gameState.player1.score >= GameState::VICTORY_CONDITION) ? 1 : 2;
    victoryEvent.player1Score = gameState.player1.score;
    victoryEvent.player2Score = gameState.player2.score;

    // Get winner's name from matchmaker
    if (matchmaker)
    {
        std::string winnerName =
            (victoryEvent.winningPlayer == 1) ? matchmaker->getPlayer1Name() : matchmaker->getPlayer2Name();
        strncpy(victoryEvent.winnerName, winnerName.c_str(), sizeof(victoryEvent.winnerName) - 1);
        victoryEvent.winnerName[sizeof(victoryEvent.winnerName) - 1] = '\0';
    }

    std::vector<uint8_t> packet =
        createPacket(MessageType::VICTORY_EVENT, gameState.frame, &victoryEvent, sizeof(victoryEvent));

    // Broadcast victory event
    if (networkManager)
    {
        networkManager->broadcastToAllClients(packet);
    }

    // Reset game after a short delay
    std::this_thread::sleep_for(std::chrono::seconds(3));
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