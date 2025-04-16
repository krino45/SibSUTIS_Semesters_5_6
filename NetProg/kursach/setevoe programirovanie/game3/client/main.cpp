// client/main.cpp
#include "../common/game_state.h"
#include "../common/network.h"
#include "game.h"
#include "input.h"
#include "network.h"
#include "render.h"
#include <iostream>
#include <thread>

int main()
{
    while (true)
    {

        pong::InputHandler inputHandler;
        pong::Renderer renderer;
        pong::NetworkManager networkManager;
        pong::Game game(inputHandler, renderer, networkManager);
        srand(time(NULL));

        if (!inputHandler.initialize() || !renderer.initialize())
        {
            std::cerr << "Failed to initialize input or renderer." << std::endl;
            return 1;
        }

        inputHandler.disableRawMode();

        std::cout << "PONG GAME" << std::endl;
        std::cout << "1. Single Player" << std::endl;
        std::cout << "2. Multiplayer (Host)" << std::endl;
        std::cout << "3. Multiplayer (Join)" << std::endl;
        std::cout << "9. Quit" << std::endl;
        std::cout << "Select mode: ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1")
        {
            // Single player mode
            game.setGameMode(pong::GameMode::LOCAL);
        }
        else if (choice == "2" || choice == "3")
        {
            // Multiplayer mode
            game.setGameMode(pong::GameMode::ONLINE);

            std::string serverAddress;
            std::string username;

            std::cout << "Enter username: ";
            std::getline(std::cin, username);

            std::cout << "Enter server address: ";
            std::getline(std::cin, serverAddress);
            if (serverAddress.size() <= 0)
            {
                serverAddress = "127.0.0.1";
            }

            if (choice == "2")
                game.setIsPlayer1(true);
            else
                game.setIsPlayer1(false);

            if (!networkManager.connectToServer(serverAddress, 8081 + rand() % 1000, 8082 + rand() % 1000, username,
                                                1000))
            {
                std::cerr << "Failed to connect to server." << std::endl;
                return 1;
            }

            std::cout << "Connected to server. Waiting for game to start..." << std::endl;
        }
        else if (choice == "9")
        {
            std::exit(0);
        }
        else
        {
            std::cerr << "Invalid choice." << std::endl;
            return 1;
        }

        networkManager.onMatchFound = [&](const pong::ConnectResponse &response) { game.setOpponentInfo(response); };

        while (true && choice != "1")
        {
            if (game.ready)
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(32));
        }

        game.start();

        while (game.running)
        {
            game.update();
            renderer.renderGameState(game.getGameState());

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    return 0;
}
