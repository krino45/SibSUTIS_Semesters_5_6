// server/game_manager.h
#pragma once
#include "game_instance.h"
#include "matchmaker.h"
#include "network.h"
#include <memory>
#include <mutex>
#include <unordered_map>

namespace pong
{
class GameInstance;
class NetworkManager;
class Matchmaker;

class GameManager
{
  public:
    GameManager();
    ~GameManager();

    void setNetworkManager(NetworkManager *networkManager)
    {
        this->networkManager = networkManager;
    }
    void setMatchmaker(Matchmaker *matchmaker)
    {
        this->matchmaker = matchmaker;
    }

    uint32_t createGame(const std::string &player1, const std::string &player2, bool start);
    GameInstance *getGame(uint32_t gameId);
    void removeGame(uint32_t gameId);
    void updateAllGames();
    void cleanupInactiveGames();
    uint32_t findGameIdForClient(const std::string &clientId);

  private:
    std::unordered_map<uint32_t, std::unique_ptr<GameInstance>> games_;
    uint32_t nextGameId_;
    std::mutex gamesMutex_;
    Matchmaker *matchmaker;
    NetworkManager *networkManager;
};

} // namespace pong