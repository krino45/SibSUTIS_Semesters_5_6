// client/main.cpp
#include "../common/game_state.h"
#include "../common/network.h"
#include "../common/utils.h"
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

        std::cout << "\nPONG GAME" << std::endl;
        std::cout << "1. Single Player" << std::endl;
        std::cout << "2. Local multiplayer" << std::endl;
        std::cout << "3. Multiplayer (Host)" << std::endl;
        std::cout << "4. Multiplayer (Join)" << std::endl;
        std::cout << "9. (Q)uit" << std::endl;
        std::cout << "Select mode: ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1")
        {
            game.setGameMode(pong::GameMode::LOCAL);
        }
        else if (choice == "2")
        {
            game.setGameMode(pong::GameMode::LOCALMULTIPLAYER);
        }
        else if (choice == "3" || choice == "4")
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

            if (choice == "3")
                game.setIsPlayer1(true);
            else
                game.setIsPlayer1(false);

            if (!networkManager.connectToServer(serverAddress, 8081 + rand() % 1000, 8082 + rand() % 1000, username,
                                                1000))
            {
                std::cerr << "Failed to connect to server." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                pong::terminal::clearScreen();
                continue;
            }

            std::cout << "Connected to server. Waiting for game to start..." << std::endl;
        }
        else if (choice == "9" || choice == "q" || choice == "Q")
        {
            std::exit(0);
        }
        else
        {
            std::cerr << "Invalid choice." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            pong::terminal::clearScreen();
            continue;
        }

        networkManager.onMatchFound = [&](const pong::ConnectResponse &response) { game.setOpponentInfo(response); };
        networkManager.onScoreEvent = [&](const pong::ScoreEvent &event) { renderer.renderGoalAnimation(); };
        networkManager.onVictoryEvent = [&](const pong::VictoryEvent &event) {
            renderer.showVictoryScreen(event.winnerName, event.player1Score, event.player2Score);
            pong::ConnectResponse empty{};
            game.setOpponentInfo(empty);
        };

        networkManager.onDisconnectEvent = [&]() {
            pong::ConnectResponse empty{};
            renderer.showDisconnectMessage();
            game.setOpponentInfo(empty);
            inputHandler.forceQuit();
        };

        while (true && (choice != "1" && choice != "2"))
        {
            networkManager.processCallbacks();
            if (game.ready)
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(32));
        }

        renderer.initialize();
        game.start();

        while (game.running)
        {
            game.update();
            renderer.renderScore(game.getGameState());
            renderer.renderControls();
            renderer.renderGameState(game.getGameState());

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }

    return 0;
}
