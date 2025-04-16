// server/main.cpp
#include <iostream>
#include <thread>
#include "matchmaker.h"
#include "network.h"

int main() {
    // Initialize matchmaker and network
    pong::Matchmaker matchmaker;
    pong::NetworkManager networkManager(matchmaker);

    // Start the server
    if (!networkManager.startServer()) {
        std::cerr << "Failed to start server." << std::endl;
        return 1;
    }

    // Main server loop
    while (true) {
        // Accept new connections and handle matchmaking
        matchmaker.process();
        networkManager.process();

        // Sleep to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
