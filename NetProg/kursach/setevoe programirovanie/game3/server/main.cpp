// server/main.cpp
#include "matchmaker.h"
#include "network.h"
#include <iostream>
#include <thread>

int main()
{
    pong::Matchmaker matchmaker;
    pong::NetworkManager networkManager(matchmaker);

    if (!networkManager.startServer())
    {
        std::cerr << "Failed to start server." << std::endl;
        return 1;
    }

    std::thread matchmakingThread([&matchmaker]() {
        while (true)
        {
            matchmaker.process();
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    });

    while (true)
    {
        networkManager.process();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
    }

    matchmakingThread.join();
    return 0;
}
