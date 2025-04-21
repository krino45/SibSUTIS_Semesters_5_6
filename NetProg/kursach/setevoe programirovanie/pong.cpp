#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <cmath>

// Game constants
const int WIDTH = 200;
const int HEIGHT = 54;
const int PADDLE_HEIGHT = 12;
const float BASE_SPEED = 2.0f;
const float SPEED_INCREASE = 0.2f;
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

void handleInput(Paddle& p1, Paddle& p2, bool& running) {
    char input;
    while (read(STDIN_FILENO, &input, 1) > 0) {
        if (input == 'q' || input == 'Q') running = false;
        
        // Player 1 controls (W/S)
        if (input == p1.up_key && p1.y > 1) {
            drawPaddle(p1, true);
            p1.y--;
            drawPaddle(p1);
        }
        else if (input == p1.down_key && p1.y + PADDLE_HEIGHT < HEIGHT) {
            drawPaddle(p1, true);
            p1.y++;
            drawPaddle(p1);
        }
        
        // Player 2 controls (arrow keys)
        if (input == '\033') {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) > 0 && 
                read(STDIN_FILENO, &seq[1], 1) > 0) {
                if (seq[0] == '[') {
                    if (seq[1] == 'A' && p2.y > 1) {
                        drawPaddle(p2, true);
                        p2.y--;
                        drawPaddle(p2);
                    }
                    else if (seq[1] == 'B' && p2.y + PADDLE_HEIGHT < HEIGHT) {
                        drawPaddle(p2, true);
                        p2.y++;
                        drawPaddle(p2);
                    }
                }
            }
        }
    }
}

int main() {
    srand(time(nullptr));
    
    // Set up terminal
    enableRawMode();
    hideCursor();
    clearScreen();
    
    // Initialize game objects
    Paddle p1 = {2, HEIGHT/2 - PADDLE_HEIGHT/2, 0, 'w', 's', "\033[38;5;46m"}; // Green
    Paddle p2 = {WIDTH-3, HEIGHT/2 - PADDLE_HEIGHT/2, 0, '\0', '\0', "\033[38;5;39m"}; // Blue
    Ball ball;
    resetBall(ball, rand() % 2);
    
    // Draw initial game state
    drawArena();
    drawPaddle(p1);
    drawPaddle(p2);
    drawBall(ball);
    drawScore(p1, p2);
    drawControls();
    
    // Game loop
    bool running = true;
    while (running) {
        // Handle input
        handleInput(p1, p2, running);
        
        // Update game state
        drawBall(ball, true);
        updateBall(ball, p1, p2);
        drawBall(ball);
        
        // Update score if needed
        static int last_p1_score = -1, last_p2_score = -1;
        if (p1.score != last_p1_score || p2.score != last_p2_score) {
            drawScore(p1, p2);
            last_p1_score = p1.score;
            last_p2_score = p2.score;
            usleep(300000);
        }
        
        // Control game speed
        usleep(GAME_SPEED);
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