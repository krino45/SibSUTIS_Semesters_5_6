// client / pong_game.cpp
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <cmath>
#include <thread>
#include <mutex>
#include <atomic>
#include <limits>
#include "../server/pong_net.h"

// Game constants
const int WIDTH = 68;
const int HEIGHT = 24;
const int PADDLE_HEIGHT = 7;
const float BASE_SPEED = 0.5f;
const float SPEED_INCREASE = 0.15f;
const int GAME_SPEED = 35000;
const int INPUT_POLL_RATE = 50;

// Game objects
struct Paddle {
    int x, y;
    int score;
    char up_key, down_key;
    const char* color;
};

struct Ball {
    float x, y;
    float dx, dy;
    float speed;
    int last_x, last_y;
};

// Terminal control
void clearScreen() {
    std::cout << "\033[2J\033[H";
}

void setCursor(int x, int y) {
    std::cout << "\033[" << y << ";" << x << "H";
}

void hideCursor() {
    std::cout << "\033[?25l";
}

void showCursor() {
    std::cout << "\033[?25h";
}

void enableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void disableRawMode() {
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

// Drawing functions
void drawArena() {
    // Set background color
    std::cout << "\033[48;5;234m";
    
    // Clear the entire play area first
    for (int y = 1; y <= HEIGHT; y++) {
        setCursor(1, y);
        for (int x = 0; x < WIDTH; x++) {
            std::cout << " ";
        }
    }
    
    // Then draw borders
    // Top and bottom borders
    for (int x = 0; x < WIDTH; x++) {
        setCursor(x + 1, 1);
        std::cout << "\033[38;5;255m═";
        setCursor(x + 1, HEIGHT);
        std::cout << "═";
    }
    
    // Side borders and center line
    for (int y = 2; y < HEIGHT; y++) {
        setCursor(1, y);
        std::cout << "\033[38;5;255m║";
        setCursor(WIDTH, y);
        std::cout << "║";
        setCursor(WIDTH/2 + 1, y);
        std::cout << (y % 2 ? "\033[38;5;239m│" : " ");
    }
    
    // Corners
    setCursor(1, 1);
    std::cout << "╔";
    setCursor(WIDTH, 1);
    std::cout << "╗";
    setCursor(1, HEIGHT);
    std::cout << "╚";
    setCursor(WIDTH, HEIGHT);
    std::cout << "╝";
    
    std::cout << "\033[0m";
}

void drawPaddle(const Paddle& p, bool erase = false) {
    if (erase) {
        for (int i = 0; i < PADDLE_HEIGHT; i++) {
            setCursor(p.x + 1, p.y + i + 1);
            std::cout << " ";
        }
        return;
    }
    
    std::cout << p.color;
    for (int i = 0; i < PADDLE_HEIGHT; i++) {
        setCursor(p.x + 1, p.y + i + 1);
        if (i == 0 || i == PADDLE_HEIGHT - 1) {
            std::cout << "■";
        } else {
            std::cout << "█";
        }
    }
    std::cout << "\033[0m";
}

void drawBall(const Ball& b, bool erase = false) {
    setCursor(b.last_x + 1, b.last_y + 1);
    std::cout << " ";
    
    if (!erase) {
        setCursor(static_cast<int>(b.x) + 1, static_cast<int>(b.y) + 1);
        float speed_factor = sqrt(b.dx*b.dx + b.dy*b.dy) / BASE_SPEED;
        if (speed_factor > 1.5f) {
            std::cout << "\033[1;33;48;5;234m●\033[0m";
        } else {
            std::cout << "\033[1;93;48;5;234m◦\033[0m";
        }
    }
    std::cout.flush();
}

void drawScore(const Paddle& p1, const Paddle& p2) {
    for (int x = WIDTH/2 - 10; x <= WIDTH/2 + 10; x++) {
        setCursor(x, HEIGHT + 2);
        std::cout << "\033[48;5;236m ";
    }
    
    setCursor(WIDTH/2 - 8, HEIGHT + 2);
    std::cout << p1.color << "PLAYER 1: " << p1.score << "\033[0m";
    
    setCursor(WIDTH/2 + 2, HEIGHT + 2);
    std::cout << p2.color << "PLAYER 2: " << p2.score << "\033[0m";
}

void drawControls() {
    setCursor(2, HEIGHT + 4);
    std::cout << "\033[38;5;245mCONTROLS: \033[1;37mP1 (W/S)   P2 (↑/↓)   \033[1;31mQUIT (Q)\033[0m";
}

void drawChatArea(const std::string& opponent_name) {
    int chat_start_y = HEIGHT + 6;
    
    setCursor(2, chat_start_y - 1);
    std::cout << "\033[38;5;245m--- CHAT WITH " << opponent_name << " ---\033[0m";
    
    // Clear chat area
    for (int y = chat_start_y; y < chat_start_y + 5; y++) {
        setCursor(2, y);
        std::cout << "\033[K"; // Clear line
    }
}

// Game logic
void resetBall(Ball& ball, bool toLeft) {
    ball.x = WIDTH / 2;
    ball.y = HEIGHT / 2;
    ball.speed = BASE_SPEED;
    ball.dx = toLeft ? -ball.speed : ball.speed;
    ball.dy = ((rand() % 100) / 100.0f - 0.5f) * 0.5f;
    ball.last_x = static_cast<int>(ball.x);
    ball.last_y = static_cast<int>(ball.y);
}

void updateBall(Ball& ball, Paddle& p1, Paddle& p2) {
    ball.last_x = static_cast<int>(ball.x);
    ball.last_y = static_cast<int>(ball.y);
    
    ball.x += ball.dx;
    ball.y += ball.dy;
    
    if (ball.y <= 1.2f) {
        ball.y = 1.2f;
        ball.dy = fabs(ball.dy) * 0.95f;
    } 
    else if (ball.y >= HEIGHT - 1.2f) {
        ball.y = HEIGHT - 1.2f;
        ball.dy = -fabs(ball.dy) * 0.95f;
    }
    
    auto checkPaddle = [&ball](const Paddle& p) {
        return ball.x >= p.x - 1.5f && ball.x <= p.x + 1.5f &&
               ball.y >= p.y - 0.5f && ball.y < p.y + PADDLE_HEIGHT + 0.5f;
    };
    
    if (checkPaddle(p1)) {
        ball.speed = std::min(ball.speed + SPEED_INCREASE, BASE_SPEED * 2.0f);
        float hitPos = (ball.y - p1.y) / PADDLE_HEIGHT - 0.5f;
        ball.dx = fabs(ball.dx);
        ball.dy = hitPos * 1.3f;
        ball.x = p1.x + 1.6f;
    } 
    else if (checkPaddle(p2)) {
        ball.speed = std::min(ball.speed + SPEED_INCREASE, BASE_SPEED * 2.0f);
        float hitPos = (ball.y - p2.y) / PADDLE_HEIGHT - 0.5f;
        ball.dx = -fabs(ball.dx);
        ball.dy = hitPos * 1.3f;
        ball.x = p2.x - 1.6f;
    }
    
    float mag = sqrt(ball.dx*ball.dx + ball.dy*ball.dy);
    if (mag > 0) {
        ball.dx = (ball.dx/mag) * ball.speed;
        ball.dy = (ball.dy/mag) * ball.speed;
    }
    
    if (ball.x < 0 || ball.x >= WIDTH) {
        if (ball.x < 0) p2.score++;
        else p1.score++;
        
        for (int i = 0; i < 3; i++) {
            setCursor(WIDTH/2 - 3, HEIGHT/2);
            std::cout << "\033[1;5;37;48;5;196m GOAL! \033[0m";
            std::cout.flush();
            usleep(150000);
            setCursor(WIDTH/2 - 3, HEIGHT/2);
            std::cout << "       ";
            std::cout.flush();
            usleep(150000);
        }
        
        resetBall(ball, ball.x < 0);
    }
}

// Chat system globals
std::mutex chat_mutex;
std::vector<std::string> chat_messages;
std::atomic<bool> chat_input_active(false);
std::string current_chat_input;

// Function to display chat messages
void displayChatMessages() {
    std::lock_guard<std::mutex> lock(chat_mutex);
    int chat_start_y = HEIGHT + 6;
    
    // Display up to the last 5 messages
    int start_idx = std::max(0, static_cast<int>(chat_messages.size()) - 5);
    for (int i = 0; i < 5 && (start_idx + i) < chat_messages.size(); i++) {
        setCursor(2, chat_start_y + i);
        std::cout << "\033[K" << chat_messages[start_idx + i]; // Clear line and print message
    }
    
    // Display current input if active
    if (chat_input_active) {
        setCursor(2, chat_start_y + 5);
        std::cout << "\033[K> " << current_chat_input;
    }
}

void chatReceiveThread(NetworkManager& network, const std::string& opponent_name) {
    while (true) {
        std::string message;
        if (network.receiveChatMessage(message)) {
            std::lock_guard<std::mutex> lock(chat_mutex);
            chat_messages.push_back(opponent_name + ": " + message);
            if (chat_messages.size() > 5) {
                chat_messages.erase(chat_messages.begin());
            }
            displayChatMessages();
        }
        usleep(100000); // Slightly longer sleep to reduce CPU usage
    }
}

// Thread for handling chat input
void chatInputThread(NetworkManager& network, const std::string& my_name) {
    std::string input;
    
    while (true) {
        // Check if we should activate chat mode
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 't' || c == 'T') { // 't' key activates chat
                // Activate chat input mode
                chat_input_active = true;
                current_chat_input = "";
                displayChatMessages();
                
                // Temporarily disable non-blocking mode for stdin
                int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
                fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
                
                // Get player's input
                setCursor(4, HEIGHT + 11);
                std::string message;
                std::getline(std::cin, message);
                
                if (!message.empty()) {
                    // Send and display the message
                    network.sendChatMessage(message);
                    {
                        std::lock_guard<std::mutex> lock(chat_mutex);
                        chat_messages.push_back(my_name + ": " + message);
                    }
                    displayChatMessages();
                }
                
                // Restore non-blocking mode
                fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
                chat_input_active = false;
            }
        }
        
        usleep(10000); // Short sleep to avoid CPU hogging
    }
}

void handleInput(Paddle& p1, Paddle& p2, bool& running, bool online, bool isPlayer1) {
    char input;
    while (read(STDIN_FILENO, &input, 1) > 0) {
        if (input == 'q' || input == 'Q') running = false;
        
        // Only process paddle movements if we control that paddle
        if (online) {
            if (isPlayer1 && (input == 'w' || input == 's')) {
                // Player 1 controls (W/S) - only process if we are player 1
                if (input == 'w' && p1.y > 1) {
                    drawPaddle(p1, true);
                    p1.y--;
                    drawPaddle(p1);
                }
                else if (input == 's' && p1.y + PADDLE_HEIGHT < HEIGHT) {
                    drawPaddle(p1, true);
                    p1.y++;
                    drawPaddle(p1);
                }
            }
            else if (!isPlayer1 && input == '\033') {
                // Player 2 controls (arrow keys) - only process if we are player 2
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) > 0 && 
                    read(STDIN_FILENO, &seq[1], 1) > 0) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'A' && p2.y > 1) { // Up arrow
                            drawPaddle(p2, true);
                            p2.y--;
                            drawPaddle(p2);
                        }
                        else if (seq[1] == 'B' && p2.y + PADDLE_HEIGHT < HEIGHT) { // Down arrow
                            drawPaddle(p2, true);
                            p2.y++;
                            drawPaddle(p2);
                        }
                    }
                }
            }
        } else {
            // Local mode - process all inputs
            if (input == 'w' && p1.y > 1) {
                drawPaddle(p1, true);
                p1.y--;
                drawPaddle(p1);
            }
            else if (input == 's' && p1.y + PADDLE_HEIGHT < HEIGHT) {
                drawPaddle(p1, true);
                p1.y++;
                drawPaddle(p1);
            }
            else if (input == '\033') { // ESC sequence
                char seq[2];
                if (read(STDIN_FILENO, &seq[0], 1) > 0 && 
                    read(STDIN_FILENO, &seq[1], 1) > 0) {
                    if (seq[0] == '[') {
                        if (seq[1] == 'A' && p2.y > 1) { // Up arrow
                            drawPaddle(p2, true);
                            p2.y--;
                            drawPaddle(p2);
                        }
                        else if (seq[1] == 'B' && p2.y + PADDLE_HEIGHT < HEIGHT) { // Down arrow
                            drawPaddle(p2, true);
                            p2.y++;
                            drawPaddle(p2);
                        }
                    }
                }
            }
        }
    }
}


int main() {
    srand(time(nullptr));
    // Set up terminal
    hideCursor();
    clearScreen();
    
    // Initialize game objects
    Paddle p1 = {2, HEIGHT/2 - PADDLE_HEIGHT/2, 0, 'w', 's', "\033[38;5;46m"};
    Paddle p2 = {WIDTH-3, HEIGHT/2 - PADDLE_HEIGHT/2, 0, '\0', '\0', "\033[38;5;39m"};
    Ball ball;
    resetBall(ball, rand() % 2);
    
    // Draw initial game state
    drawArena();
    drawPaddle(p1);
    drawPaddle(p2);
    drawBall(ball);
    drawScore(p1, p2);
    drawControls();
    
    GameMode mode = LOCAL;
    NetworkManager network;
    bool isPlayer1 = true; // Default value, will be assigned later for online mode
    std::string my_username = "Player";
    
    // Add menu to choose game mode
    while (true) {
        clearScreen();
        std::cout << "Choose game mode:\n";
        std::cout << "1. Local 2-player\n";
        std::cout << "2. Online multiplayer\n";
        
        std::string input;
        std::getline(std::cin, input);
        
        if (input == "1") {
            mode = LOCAL;
            break;
        } else if (input == "2") {
            mode = ONLINE;
            break;
        } else {
            std::cout << "Invalid choice. Please enter 1 or 2.\n";
            sleep(1);
        }
    }
    
    if (mode == ONLINE) {        
        std::cout << "Enter username: ";
        std::cin >> my_username;
        
        std::string server_ip;
        std::cout << "Enter server IP (default: 127.0.0.1): ";
        std::cin.ignore();
        std::getline(std::cin, server_ip);
        if (server_ip.empty()) server_ip = "127.0.0.1";
        
        std::cout << "Connecting to server and finding opponent...\n";
        
        // Generate random port numbers for this client
        int udp_port = 8081 + (rand() % 1000);
        int tcp_port = 8082 + (rand() % 1000);
        
        if (!network.connectToServer(server_ip, 8080, my_username, 1000, udp_port, tcp_port)) {
            std::cerr << "Failed to connect to server or find match\n";
            disableRawMode();
            showCursor();
            return 1;
        }
        
        std::cout << "Matched with: " << network.getOpponentUsername() << "\n";
        std::cout << "Starting game...\n";
        
        // Decide which player we are based on usernames (simple but consistent)
        isPlayer1 = my_username < network.getOpponentUsername();
        
        sleep(2);
        
        // Redraw game after menu
        clearScreen();
        drawArena();
        drawPaddle(p1);
        drawPaddle(p2);
        drawBall(ball);
        drawScore(p1, p2);
        drawControls();
        
        // Setup chat system
        std::string opponent_name = network.getOpponentUsername();
        drawChatArea(opponent_name);
        
        // Start chat threads
        std::thread chat_receiver(chatReceiveThread, std::ref(network), opponent_name);
        chat_receiver.detach();
        
        std::thread chat_input(chatInputThread, std::ref(network), my_username);
        chat_input.detach();
        
        // Add chat instructions
        setCursor(2, HEIGHT + 5);
        std::cout << "\033[38;5;245mPress 'T' to chat\033[0m";
    }

    enableRawMode();
    // Game loop
bool running = true;
auto last_frame = std::chrono::steady_clock::now();
const auto frame_duration = std::chrono::microseconds(16667); // ~60fps

while (running) {
    auto frame_start = std::chrono::steady_clock::now();
    
    // Handle input for local controls
    handleInput(p1, p2, running, mode == ONLINE, isPlayer1);
    
    if (mode == ONLINE) {
        // Send our paddle position (and ball if we're player 1)
        if (isPlayer1) {
            network.sendGameState(ball.x, ball.y, ball.dx, ball.dy, 
                                p1.y, p2.y, p1.score, p2.score);
        } else {
            // Just send our paddle position
            network.sendGameState(0, 0, 0, 0, 0, p2.y, 0, 0);
        }
        
        // Try to receive game state
        float opp_ball_x, opp_ball_y, opp_ball_dx, opp_ball_dy;
        int opp_p1_y, opp_p2_y, opp_p1_score, opp_p2_score;
        if (network.receiveGameState(opp_ball_x, opp_ball_y, opp_ball_dx, opp_ball_dy,
                                opp_p1_y, opp_p2_y, opp_p1_score, opp_p2_score)) {
            // Update game state based on received data
            if (isPlayer1) {
                // We're player 1, only update player 2's paddle
                p2.y = opp_p2_y;
            } else {
                // We're player 2, update ball and player 1's paddle
                p1.y = opp_p1_y;
                ball.x = opp_ball_x;
                ball.y = opp_ball_y;
                ball.dx = opp_ball_dx;
                ball.dy = opp_ball_dy;
                p1.score = opp_p1_score;
                p2.score = opp_p2_score;
            }
            
            drawScore(p1, p2);
        }
    }
    
    // Update game state based on who controls the ball
    drawBall(ball, true);
    
    if (mode == LOCAL || (mode == ONLINE && isPlayer1)) {
        updateBall(ball, p1, p2);
    }
    
    drawBall(ball);
    
    // Update score if needed
    static int last_p1_score = -1, last_p2_score = -1;
    if (p1.score != last_p1_score || p2.score != last_p2_score) {
        drawScore(p1, p2);
        last_p1_score = p1.score;
        last_p2_score = p2.score;
    }
    
    // Calculate frame time and sleep only the remaining time
    auto frame_end = std::chrono::steady_clock::now();
    auto frame_time = frame_end - frame_start;
    auto sleep_time = frame_duration - frame_time;
    
    if (sleep_time > std::chrono::microseconds::zero()) {
        usleep(std::chrono::duration_cast<std::chrono::microseconds>(sleep_time).count());
    }
}
    
    // Clean up
    showCursor();
    disableRawMode();
    clearScreen();
    
    // Game over message
    std::cout << "\n  \033[1;36mGAME OVER\033[0m\n\n";
    std::cout << "  \033[1;32mPlayer 1: " << p1.score << "\033[0m\n";
    std::cout << "  \033[1;34mPlayer 2: " << p2.score << "\033[0m\n\n";
    
    return 0;
}