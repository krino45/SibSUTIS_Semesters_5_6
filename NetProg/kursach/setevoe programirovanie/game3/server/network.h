// server/network.h

#pragma once

#include "../common/network.h"
#include "matchmaker.h"
#include <arpa/inet.h>
#include <condition_variable>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

namespace pong
{

class NetworkManager
{
  public:
    NetworkManager(Matchmaker &matchmaker);
    ~NetworkManager();

    bool startServer();
    void process();

  private:
    Matchmaker &matchmaker;
    int serverSocket;
    std::thread acceptThread;
    std::unordered_map<std::string, int> clientSockets;
    std::mutex mutex;

    void acceptConnections();
    void handleClient(int clientSocket, const sockaddr_in &clientAddr);
};

} // namespace pong
