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
    if (!running)
    {
        std::exit(1);
    }
    running = false;
}

int main()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    pong::Matchmaker matchmaker;
    pong::GameManager gameManager;
    pong::NetworkManager networkManager;

    networkManager.setGameManager(&gameManager);
    networkManager.setMatchmaker(&matchmaker);

    matchmaker.setGameManager(&gameManager);
    matchmaker.setNetworkManager(&networkManager);

    gameManager.setMatchmaker(&matchmaker);
    gameManager.setNetworkManager(&networkManager);

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
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    networkManager.shutdown();

    std::cout << "Server shutdown complete." << std::endl;
    return 0;
}