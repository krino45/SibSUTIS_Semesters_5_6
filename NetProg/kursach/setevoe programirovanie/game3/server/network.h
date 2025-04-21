// server/network.h
#pragma once
#include "../common/network.h"

#include "game_instance.h"
#include "game_manager.h"
#include "matchmaker.h"

#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

namespace pong
{

// Forward declaration
class Matchmaker;
class GameManager;
class GameInstance;

struct ConnectedClient
{
    std::string clientId;      // Unique identifier for the client (IP:port)
    uint8_t playerId;          // Player ID in the game (1 or 2)
    std::string address;       // IP address
    uint16_t port;             // UDP port
    uint32_t lastActivityTime; // For timeout detection
};

class NetworkManager
{
  public:
    NetworkManager();
    ~NetworkManager();

    bool startServer(uint16_t port = UDP_SERVER_PORT);
    void process();
    void shutdown();
    size_t getClientCount() const
    {
        return clients.size();
    }

    // Methods for sending data to clients
    void sendToClient(const std::string &clientId, const std::vector<uint8_t> &packet);
    void sendToClient(const std::string &address, uint16_t port, const std::vector<uint8_t> &packet);
    void broadcastToGame(const std::vector<uint8_t> &packet, uint32_t gameId);

    // Set the matchmaker reference
    void setMatchmaker(Matchmaker *matchmaker)
    {
        this->matchmaker = matchmaker;
    }

    void setGameManager(GameManager *manager)
    {
        this->gameManager = manager;
    }

    uint32_t findGameIdForClient(const std::string &clientId);

  private:
    void receiveLoop();
    void handlePacket(const std::vector<uint8_t> &data, const sockaddr_in &sender);
    void handleConnectRequest(const std::vector<uint8_t> &data, const sockaddr_in &sender);
    void handleClientDisconnect(const std::string &clientId, bool notifyOthers);
    void handlePlayerInput(const std::vector<uint8_t> &data, const std::string &clientId);

    std::string getClientIdentifier(const sockaddr_in &addr);

    // Socket and thread management
    int udpSocket;
    std::thread receiveThread;
    bool running;

    // Client management
    std::mutex clientsMutex;
    std::vector<ConnectedClient> clients;
    std::unordered_map<std::string, size_t> clientIdToIndex;

    std::chrono::steady_clock::time_point lastUpdate;
    std::chrono::steady_clock::time_point lastCleanupTime;

    // Reference to the matchmaker
    Matchmaker *matchmaker;

    GameManager *gameManager;
};

} // namespace pong