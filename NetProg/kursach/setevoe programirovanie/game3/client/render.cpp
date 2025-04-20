// client/render.cpp
#include "render.h"
#include "../common/network.h"
#include "../common/utils.h"
#include <chrono>
#include <iomanip>
#include <thread>

namespace pong
{

Renderer::Renderer() : lastBallX(0), lastBallY(0), width(GameState::WIDTH), height(GameState::HEIGHT)
{
}

Renderer::~Renderer()
{
    // Reset terminal on exit
    terminal::showCursor();
    terminal::resetColor();
    std::cout << std::endl;
}

bool Renderer::initialize()
{
    // Set up terminal
    terminal::clearScreen();
    terminal::hideCursor();

    // Draw initial UI elements
    drawArena();

    return true;
}

void Renderer::clearScreen()
{
    terminal::clearScreen();
}

void Renderer::drawArena()
{
    // Set background color (dark gray)
    std::cout << "\033[48;5;234m";

    // Clear the entire play area first
    for (int y = 1; y <= height; y++)
    {
        terminal::setCursor(1, y);
        for (int x = 0; x < width; x++)
        {
            std::cout << " ";
        }
    }

    // Draw borders (white)
    std::cout << "\033[38;5;255m";

    // Top and bottom borders
    for (int x = 0; x < width; x++)
    {
        terminal::setCursor(x + 1, 1);
        std::cout << "═";
        terminal::setCursor(x + 1, height);
        std::cout << "═";
    }

    // Side borders and center line
    for (int y = 2; y < height; y++)
    {
        terminal::setCursor(1, y);
        std::cout << "║";
        terminal::setCursor(width, y);
        std::cout << "║";

        // Dotted center line (dark gray)
        terminal::setCursor(width / 2 + 1, y);
        std::cout << (y % 2 ? "\033[38;5;239m│" : " ");
    }

    // Draw corners
    terminal::setCursor(1, 1);
    std::cout << "╔";
    terminal::setCursor(width, 1);
    std::cout << "╗";
    terminal::setCursor(1, height);
    std::cout << "╚";
    terminal::setCursor(width, height);
    std::cout << "╝";

    // Reset colors
    terminal::resetColor();
}

void Renderer::drawPaddle(const Paddle &paddle, bool erase)
{
    const int paddleX = static_cast<int>(paddle.position.x + 0.5f);
    const int paddleY = static_cast<int>(paddle.position.y + 0.5f);

    // Set the appropriate color or erase
    if (erase)
    {
        std::cout << "\033[48;5;234m"; // Background color
    }
    else if (paddleX < width / 2)
    {
        std::cout << "\033[38;5;46m"; // Green for left paddle
    }
    else
    {
        std::cout << "\033[38;5;39m"; // Blue for right paddle
    }

    // Draw the paddle
    for (int i = 0; i < Paddle::HEIGHT; i++)
    {
        terminal::setCursor(paddleX + 1, paddleY + i + 1);
        if (erase)
        {
            std::cout << " ";
        }
        else if (i == 0 || i == Paddle::HEIGHT - 1)
        {
            std::cout << "■"; // Top and bottom
        }
        else
        {
            std::cout << "█"; // Middle sections
        }
    }

    terminal::resetColor();
}

void Renderer::drawBall(const Ball &ball, bool erase)
{
    // Round ball position to integer coordinates
    int ballX = static_cast<int>(ball.position.x + 0.5f);
    int ballY = static_cast<int>(ball.position.y + 0.5f);

    // Erase the previous position if different
    if (lastBallX != ballX || lastBallY != ballY)
    {
        terminal::setCursor(lastBallX + 1, lastBallY + 1);
        std::cout << "\033[48;5;234m " << std::flush; // Erase with background color
    }

    // Draw the new ball position if not erasing
    if (!erase)
    {
        terminal::setCursor(ballX + 1, ballY + 1);

        // Change appearance based on speed
        float speed = ball.velocity.magnitude();
        if (speed > 0.75f)
        {
            std::cout << "\033[1;33;48;5;234m●" << std::flush; // Fast - yellow, solid
        }
        else
        {
            std::cout << "\033[1;93;48;5;234m◦" << std::flush; // Normal - light yellow, hollow
        }

        // Remember this position for later erasing
        lastBallX = ballX;
        lastBallY = ballY;
    }

    terminal::resetColor();
}

void Renderer::drawChatArea()
{
    for (int y = height + 6; y < height + 11; ++y)
    {
        terminal::setCursor(2, y);
        std::cout << "                    ";
    }
}

void Renderer::renderScore(const GameState &state)
{
    terminal::setCursor(width / 2 - 10, height + 2);
    std::cout << getColoredText("PLAYER 1: " + std::to_string(state.player1.score), 46);
    terminal::setCursor(width / 2 + 2, height + 2);
    std::cout << getColoredText("PLAYER 2: " + std::to_string(state.player2.score), 39);
}

void Renderer::renderControls()
{
    terminal::setCursor(2, height + 4);
    std::cout << getColoredText("CONTROLS: P1 (W/S)   P2 (↑/↓)   QUIT (Q)", 245);
}

void Renderer::renderChatInput(const std::string &inputText)
{
    terminal::setCursor(2, height + 11);
    std::cout << "> " << inputText;
}

void Renderer::initializeChatArea(const std::string &opponentName)
{
    terminal::setCursor(2, height + 6);
    std::cout << getColoredText("--- CHAT WITH " + opponentName + " ---", 245);
}

void Renderer::renderGoalAnimation()
{
    terminal::setCursor(width / 2 - 3, height / 2);
    std::cout << terminal::colorText("GOAL!", 196, 155);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    initialize();
}

void Renderer::showMatchFoundAnimation(const std::string &opponentName)
{
    // Clear the middle of the screen
    for (int y = height / 2 - 3; y <= height / 2 + 3; y++)
    {
        terminal::setCursor(width / 4, y);
        std::cout << std::string(width / 2, ' ');
    }

    // Animated countdown
    std::string message = "MATCH FOUND: " + opponentName;

    // Flash the message
    for (int i = 0; i < 3; i++)
    {
        // Show message
        terminal::setCursor(width / 2 - message.length() / 2, height / 2 - 1);
        std::cout << getColoredText(message, 226); // Bright yellow

        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        // Hide message
        terminal::setCursor(width / 2 - message.length() / 2, height / 2 - 1);
        std::cout << std::string(message.length(), ' ');

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Show message one final time
    terminal::setCursor(width / 2 - message.length() / 2, height / 2 - 1);
    std::cout << getColoredText(message, 226);

    // Countdown animation
    for (int i = 3; i > 0; i--)
    {
        terminal::setCursor(width / 2, height / 2 + 1);
        std::cout << getColoredText("Starting in " + std::to_string(i) + "...", 245);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Clear the animation
    for (int y = height / 2 - 3; y <= height / 2 + 3; y++)
    {
        terminal::setCursor(width / 4, y);
        std::cout << std::string(width / 2, ' ');
    }

    // Re-draw the arena since we might have overwritten parts of it
    drawArena();
}

void Renderer::showVictoryScreen(const std::string &winnerName, int player1Score, int player2Score)
{
    // Save current terminal state
    terminal::showCursor();
    terminal::clearScreen();

    // Create box for victory message
    int boxWidth = 40;
    int boxHeight = 10;
    int boxX = (width - boxWidth) / 2;
    int boxY = (height - boxHeight) / 2;

    // Draw box
    std::cout << "\033[38;5;220m"; // Gold color

    // Using ASCII characters instead of Unicode box drawing characters
    // Top border
    terminal::setCursor(boxX, boxY);
    std::cout << "+";
    for (int i = 0; i < boxWidth - 2; i++)
    {
        std::cout << "-";
    }
    std::cout << "+";

    // Side borders
    for (int y = 1; y < boxHeight - 1; y++)
    {
        terminal::setCursor(boxX, boxY + y);
        std::cout << "|";
        terminal::setCursor(boxX + boxWidth - 1, boxY + y);
        std::cout << "|";
    }

    // Bottom border
    terminal::setCursor(boxX, boxY + boxHeight - 1);
    std::cout << "+";
    for (int i = 0; i < boxWidth - 2; i++)
    {
        std::cout << "-";
    }
    std::cout << "+";

    // Title
    terminal::setCursor(boxX + (boxWidth - 12) / 2, boxY + 2);
    std::cout << "\033[1;38;5;220mGAME OVER!\033[0m";

    // Winner message
    std::string message = winnerName + " wins!";
    terminal::setCursor(boxX + (boxWidth - message.length()) / 2, boxY + 4);
    std::cout << getColoredText(message, winnerName == "PLAYER 1" ? 46 : 39);

    // Final score
    terminal::setCursor(boxX + (boxWidth - 15) / 2, boxY + 6);
    std::cout << getColoredText(std::to_string(player1Score) + " - " + std::to_string(player2Score), 255);

    // Press any key to continue
    terminal::setCursor(boxX + (boxWidth - 26) / 2, boxY + 8);
    std::cout << getColoredText("Press any key to continue", 245);

    // Reset color
    terminal::resetColor();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Clear the victory screen
    clearScreen();
    drawArena();

    // Hide cursor again
    terminal::hideCursor();
}

void Renderer::showDisconnectMessage()
{
    // Create a box in the middle of the screen
    int boxWidth = 30;
    int boxHeight = 5;
    int boxX = (width - boxWidth) / 2;
    int boxY = (height - boxHeight) / 2;

    // Draw box with red border
    std::cout << "\033[38;5;196m"; // Red color

    // Top border
    terminal::setCursor(boxX, boxY);
    std::cout << "+";
    for (int i = 0; i < boxWidth - 2; i++)
    {
        std::cout << "-";
    }
    std::cout << "+";

    // Side borders
    for (int y = 1; y < boxHeight - 1; y++)
    {
        terminal::setCursor(boxX, boxY + y);
        std::cout << "|";
        terminal::setCursor(boxX + boxWidth - 1, boxY + y);
        std::cout << "|";
    }

    // Bottom border
    terminal::setCursor(boxX, boxY + boxHeight - 1);
    std::cout << "+";
    for (int i = 0; i < boxWidth - 2; i++)
    {
        std::cout << "-";
    }
    std::cout << "+";

    // Disconnect message
    std::string message = "OPPONENT DISCONNECTED";
    terminal::setCursor(boxX + (boxWidth - message.length()) / 2, boxY + 2);
    std::cout << getColoredText(message, 196); // Red text

    // Animation: Flash the message a few times
    for (int i = 0; i < 5; i++)
    {
        terminal::setCursor(boxX + (boxWidth - message.length()) / 2, boxY + 2);
        if (i % 2 == 0)
        {
            std::cout << std::string(message.length(), ' ');
        }
        else
        {
            std::cout << getColoredText(message, 196);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    // Show message one final time
    terminal::setCursor(boxX + (boxWidth - message.length()) / 2, boxY + 2);
    std::cout << getColoredText(message, 196);

    // Wait for a few seconds
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // Reset color
    terminal::resetColor();
}

void Renderer::renderGameState(const GameState &state, float interpolation)
{
    GameState interpolatedState = interpolateStates(prevState, state, interpolation);
    drawPaddle(prevState.player1, true);
    drawPaddle(prevState.player2, true);
    drawBall(prevState.ball, true);
    drawPaddle(interpolatedState.player1);
    drawPaddle(interpolatedState.player2);
    drawBall(interpolatedState.ball);
    prevState = state;
}

void Renderer::renderChatMessages(const std::vector<ChatMessageData> &messages)
{
    drawChatArea();
    int startY = height + 6;
    for (size_t i = 0; i < messages.size() && i < 5; ++i)
    {
        terminal::setCursor(2, startY + i);
        std::cout << messages[i].sender << ": " << messages[i].content;
    }
}

std::string Renderer::getColoredText(const std::string &text, int color) const
{
    return terminal::colorText(text, color);
}

GameState Renderer::interpolateStates(const GameState &prev, const GameState &current, float alpha)
{
    GameState interpolated;
    interpolated.player1.position = Vec2::lerp(prev.player1.position, current.player1.position, alpha);
    interpolated.player2.position = Vec2::lerp(prev.player2.position, current.player2.position, alpha);
    interpolated.ball.position = Vec2::lerp(prev.ball.position, current.ball.position, alpha);
    interpolated.ball.velocity = Vec2::lerp(prev.ball.velocity, current.ball.velocity, alpha);
    interpolated.ball.speed = prev.ball.speed + (current.ball.speed - prev.ball.speed) * alpha;
    interpolated.frame = current.frame;
    return interpolated;
}

} // namespace pong