// server/main.cpp
#include "matchmaker.h"
#include "network.h"
#include <iostream>
#include <signal.h>
#include <thread>

volatile bool running = true;

// Signal handler
void signalHandler(int signal)
{
    std::cout << "Received interrupt / termination signal. Exiting..." << std::endl;
    running = false;
}

int main()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    pong::Matchmaker matchmaker;
    pong::ServerGame serverGame;
    pong::NetworkManager networkManager;

    networkManager.setServerGame(&serverGame);
    networkManager.setMatchmaker(&matchmaker);

    serverGame.setNetworkManager(&networkManager);
    serverGame.setMatchmaker(&matchmaker);

    matchmaker.setNetworkManager(&networkManager);

    if (!networkManager.startServer())
    {
        std::cerr << "Failed to start server." << std::endl;
        return 1;
    }

    while (running)
    {
        matchmaker.process();
        networkManager.process();

        // Sleep to avoid maxing out CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    networkManager.shutdown();

    std::cout << "Server shutdown complete." << std::endl;
    return 0;
}