// server/game_manager.cpp
#include "game_manager.h"

namespace pong
{

GameManager::GameManager() : nextGameId_(1), networkManager(nullptr), matchmaker(nullptr)
{
}

GameManager::~GameManager()
{
}

GameInstance *GameManager::getGame(uint32_t gameId)
{
    std::lock_guard<std::mutex> lock(gamesMutex_);
    auto it = games_.find(gameId);
    if (it != games_.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void GameManager::removeGame(uint32_t gameId)
{
    std::lock_guard<std::mutex> lock(gamesMutex_);
    games_.erase(gameId);
}

uint32_t GameManager::createGame(const std::string &player1, const std::string &player2, bool start = true)
{
    std::lock_guard<std::mutex> lock(gamesMutex_);
    uint32_t gameId = nextGameId_++;
    games_[gameId] = std::make_unique<GameInstance>(gameId, player1, player2);
    games_[gameId].get()->setMatchmaker(matchmaker);
    games_[gameId].get()->setNetworkManager(networkManager);
    if (start)
        games_[gameId].get()->startGame();
    return gameId;
}

void GameManager::updateAllGames()
{
    std::lock_guard<std::mutex> lock(gamesMutex_);
    for (auto &[id, game] : games_)
    {
        if (game->isActive())
        {
            game->update();
        }
    }
}

void GameManager::cleanupInactiveGames()
{
    std::lock_guard<std::mutex> lock(gamesMutex_);
    for (auto it = games_.begin(); it != games_.end();)
    {
        if (!it->second->isActive())
        {
            it = games_.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

uint32_t GameManager::findGameIdForClient(const std::string &clientId)
{
    std::lock_guard<std::mutex> lock(gamesMutex_);
    for (const auto &[id, game] : games_)
    {
        if (game->hasPlayer(clientId))
        {
            return id;
        }
    }
    return 0;
}

} // namespace pong