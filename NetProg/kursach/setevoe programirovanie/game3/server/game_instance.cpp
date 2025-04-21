// server/game_instance.cpp
#include "game_instance.h"
#include <iostream>

namespace pong
{

GameInstance::GameInstance(uint32_t id, const std::string &player1, const std::string &player2)
    : id_(id), active_(true), player1Id_(player1), player2Id_(player2), networkManager_(nullptr), matchmaker_(nullptr),
      frameCounter_(0)
{
    /*
    GameState gameState_;
    std::vector<PlayerInput> pendingInputs_;
    std::mutex inputMutex_; ?
    */
    gameState_.reset(rand() % 2 == 0);
}

void GameInstance::startGame()
{
    gameState_.reset(rand() % 2 == 0);
    active_ = true;
    frameCounter_ = 0;
    std::cout << "Game " << id_ << " started!" << std::endl;
}

const GameState &GameInstance::getGameState() const
{
    return gameState_;
}

bool GameInstance::isActive() const
{
    return active_;
}

void GameInstance::stopGame()
{
    active_ = false;
}

uint32_t GameInstance::getId() const
{
    return id_;
}

bool GameInstance::hasPlayer(const std::string &clientId) const
{
    return (clientId == player1Id_ || clientId == player2Id_);
}

std::vector<std::string> GameInstance::getAllPlayers() const
{
    return std::vector<std::string>{player1Id_, player2Id_};
}

void GameInstance::update()
{
    if (!active_)
        return;

    frameCounter_++;
    gameState_.frame = frameCounter_;

    // Update ball position and handle collisions
    if (gameState_.update())
    {
        // A goal was scored
        handleGoalScored();
    }

    // Process all pending player inputs
    processPlayerInputs();

    // Broadcast state to clients
    if (networkManager_ != nullptr)
    {
        broadcastState(networkManager_);
    }
    else
    {
        std::cerr << "Network manager is null @ game_instance\n";
        active_ = false;
    }
}

void GameInstance::handleGoalScored()
{
    // Create score event packet
    ScoreEvent scoreEvent;
    scoreEvent.scoringPlayer = (gameState_.lastScoringPlayerIsPlayer1) ? 1 : 2;
    scoreEvent.player1Score = gameState_.player1.score;
    scoreEvent.player2Score = gameState_.player2.score;

    std::vector<uint8_t> packet =
        createPacket(MessageType::SCORE_EVENT, gameState_.frame, &scoreEvent, sizeof(scoreEvent));

    if (networkManager_)
    {
        networkManager_->sendToClient(player1Id_, packet);
        networkManager_->sendToClient(player2Id_, packet);
    }

    if (scoreEvent.player1Score >= GameState::VICTORY_CONDITION ||
        scoreEvent.player2Score >= GameState::VICTORY_CONDITION)
    {
        handleVictory();
    }
}

void GameInstance::handleVictory()
{
    VictoryEvent victoryEvent;
    victoryEvent.winningPlayer = (gameState_.player1.score >= GameState::VICTORY_CONDITION) ? 1 : 2;
    victoryEvent.player1Score = gameState_.player1.score;
    victoryEvent.player2Score = gameState_.player2.score;

    // Get winner's name from matchmaker
    if (matchmaker_)
    {
        std::string winnerName;
        std::string loserName;
        if (victoryEvent.winningPlayer == 1)
        {
            winnerName = matchmaker_->getPlayer1Name();
            loserName = matchmaker_->getPlayer2Name();
        }
        else
        {
            winnerName = matchmaker_->getPlayer2Name();
            loserName = matchmaker_->getPlayer1Name();
        }

        matchmaker_->updateMMR(winnerName, loserName);
        strncpy(victoryEvent.winnerName, winnerName.c_str(), sizeof(victoryEvent.winnerName) - 1);
        victoryEvent.winnerName[sizeof(victoryEvent.winnerName) - 1] = '\0';
    }

    std::vector<uint8_t> packet =
        createPacket(MessageType::VICTORY_EVENT, gameState_.frame, &victoryEvent, sizeof(victoryEvent));

    // Broadcast victory event
    if (networkManager_)
    {
        networkManager_->sendToClient(player1Id_, packet);
        networkManager_->sendToClient(player2Id_, packet);
    }

    // Deactivate game
    matchmaker_->deregisterPlayer(matchmaker_->getPlayer1Name());
    matchmaker_->deregisterPlayer(matchmaker_->getPlayer2Name());

    active_ = false;
}

void GameInstance::processPlayerInputs()
{
    std::lock_guard<std::mutex> lock(inputMutex_);
    for (const auto &input : pendingInputs_)
    {
        if (input.playerId == 1)
        {
            if (input.flags & InputFlags::UP || input.flags & InputFlags::ARROW_UP)
            {
                if (gameState_.player1.position.y > 1)
                {
                    gameState_.player1.position.y -= 1;
                }
            }
            if (input.flags & InputFlags::DOWN || input.flags & InputFlags::ARROW_DOWN)
            {
                if (gameState_.player1.position.y < GameState::HEIGHT - Paddle::HEIGHT - 1)
                {
                    gameState_.player1.position.y += 1;
                }
            }
        }
        else if (input.playerId == 2)
        {
            if (input.flags & InputFlags::UP || input.flags & InputFlags::ARROW_UP)
            {
                if (gameState_.player2.position.y > 1)
                {
                    gameState_.player2.position.y -= 1;
                }
            }
            if (input.flags & InputFlags::DOWN || input.flags & InputFlags::ARROW_DOWN)
            {
                if (gameState_.player2.position.y < GameState::HEIGHT - Paddle::HEIGHT - 1)
                {
                    gameState_.player2.position.y += 1;
                }
            }
        }
    }
    pendingInputs_.clear();
}

void GameInstance::addPlayerInput(uint8_t playerId, uint8_t inputFlags)
{
    std::lock_guard<std::mutex> lock(inputMutex_);
    pendingInputs_.push_back({playerId, inputFlags});
    std::cout << "Game " << id_ << " - Received input - Player: " << (int)playerId << " Flags: " << (int)inputFlags
              << std::endl;
}

void GameInstance::broadcastState(NetworkManager *networkManager)
{
    if (!networkManager)
        return;

    GameState gamestate;
    memset(&gamestate, 0, sizeof(GameState));
    gamestate = gameState_;

    std::vector<uint8_t> packet =
        createPacket(MessageType::GAME_STATE_UPDATE, gameState_.frame, &gameState_, sizeof(GameState));

    networkManager->sendToClient(player1Id_, packet);
    networkManager->sendToClient(player2Id_, packet);
}

} // namespace pong