#pragma once
#include "../common/network.h"
#include "game.h"
#include "matchmaker.h"
#include <netinet/in.h>
#include <thread>
#include <vector>

namespace pong
{

struct ConnectedClient
{
    int socket;
    uint8_t playerId;
    std::string address;
};

class NetworkManager
{
  public:
    NetworkManager(Matchmaker &matchmaker);
    ~NetworkManager();

    bool startServer();
    void process();

  private:
    void acceptConnections();
    void handleClient(int clientSocket, const sockaddr_in &clientAddr);
    void broadcastGameState();

    Matchmaker &matchmaker;
    std::mutex queueMutex;
    ServerGame serverGame;
    int udpSocket;
    std::thread acceptThread;
    std::vector<ConnectedClient> connectedClients;
    std::chrono::steady_clock::time_point lastUpdate;
};

} // namespace pong