// client/main.cpp
#include "../common/game_state.h"
#include "../common/network.h"
#include "../common/utils.h"
#include "game.h"
#include "input.h"
#include "network.h"
#include "render.h"
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
        pong::restoreTerminal();
        std::exit(1);
    }
    running = false;
}

int main()
{
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    while (running)
    {
        pong::InputHandler inputHandler;
        pong::Renderer renderer;
        pong::NetworkManager networkManager;

        auto game = std::make_shared<pong::Game>(inputHandler, renderer, networkManager);

        // Rest of initialization...
        srand(time(NULL));

        if (!inputHandler.initialize() || !renderer.initialize())
        {
            std::cerr << "Failed to initialize input or renderer." << std::endl;
            return 1;
        }

        std::cout << "\nPONG GAME" << std::endl;
        std::cout << "1. Single Player" << std::endl;
        std::cout << "2. Local multiplayer" << std::endl;
        std::cout << "3. Multiplayer (Host)" << std::endl;
        std::cout << "4. Multiplayer (Join)" << std::endl;
        std::cout << "9. (Q)uit" << std::endl;
        std::cout << "Select mode: ";

        std::string choice;

        inputHandler.prepareForMenuInput();

        if (!std::getline(std::cin, choice))
        {
            if (std::cin.eof())
            {
                continue;
            }
            else
            {
                std::cerr << "Unknown std_in error\n";
            }
        }
        // std::cout << "[DEBUG] User entered: \"" << choice << "\"" << std::endl;

        if (choice == "1")
        {
            game->setGameMode(pong::GameMode::LOCAL);
        }
        else if (choice == "2")
        {
            game->setGameMode(pong::GameMode::LOCALMULTIPLAYER);
        }
        else if (choice == "3" || choice == "4")
        {
            // Multiplayer mode
            game->setGameMode(pong::GameMode::ONLINE);

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
                game->setIsPlayer1(true);
            else
                game->setIsPlayer1(false);

            game->udpPort = 8081 + rand() % 1000;
            game->tcpPort = 8082 + rand() % 1000;
            while (game->udpPort == game->tcpPort)
            {
                game->tcpPort = 8082 + rand() % 1000; // Making sure they dont match
            }
            if (!networkManager.connectToServer(serverAddress, game->udpPort, game->tcpPort, username))
            {
                std::cerr << "Failed to connect to server." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                pong::terminal::clearScreen();

                if (!running)
                {
                    break;
                }
                continue;
            }

            inputHandler.setChatCallback([&game]() { game->toggleChat(); });
            inputHandler.setQuitCallback([&]() {
                networkManager.stopChat();
                game->ready = false;
            });

            networkManager.onMatchFound = [&](const pong::ConnectResponse &response) {
                game->setOpponentInfo(response);
                if (response.isPlayer1)
                {
                    networkManager.startChatServer(game->tcpPort);
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                else
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    networkManager.connectToChat(response.hostAddress, response.hostTcpPort);
                }
                // std::cout << "\n\nDebug: " << response.hostAddress << response.hostTcpPort << response.hostUdpPort
                //   << response.opponentName << response.success << response.isPlayer1 << std::flush;

                std::this_thread::sleep_for(std::chrono::milliseconds(400));
                renderer.initializeChatArea(response.opponentName);

                game->getRenderer().showMatchFoundAnimation(response.opponentName, response.mmr);
            };

            networkManager.onScoreEvent = [game](const pong::ScoreEvent &event) {
                game->getRenderer().renderGoalAnimation();
            };

            networkManager.onVictoryEvent = [&](const pong::VictoryEvent &event) {
                game->getRenderer().showVictoryScreen(event.winnerName, event.player1Score, event.player2Score);
                game->running = false;
            };

            networkManager.onDisconnectEvent = [&]() {
                game->getRenderer().showDisconnectMessage();
                game->running = false;
            };

            networkManager.onChatMessage = [&](const pong::ChatMessageData &message) {
                game->addChatMessage(message);
                renderer.renderChatMessages(game->getChatMessages());
            };
            std::cout << "Connected to server. Waiting for game to start..." << std::endl;
        }
        else if (choice == "9" || choice == "q" || choice == "Q")
        {
            running = false;
            continue;
        }
        else
        {
            std::cerr << "Invalid choice." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            pong::terminal::clearScreen();

            if (!running)
            {
                break;
            }
            continue;
        }

        while (choice != "1" && choice != "2" && running)
        {
            networkManager.processCallbacks();
            if (game->ready)
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(64));
        }

        renderer.initialize();
        game->start();
        networkManager.stopChat();
    }

    return 0;
}