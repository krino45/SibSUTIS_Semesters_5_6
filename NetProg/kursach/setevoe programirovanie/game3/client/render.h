// client/render.h
#pragma once

#include "../common/game_state.h"
#include "../common/network.h"
#include <deque>
#include <iostream>
#include <string>
#include <vector>

namespace pong
{

class Renderer
{
  public:
    Renderer();
    ~Renderer();

    // Initialize rendering system
    bool initialize();

    // Clear the screen
    void clearScreen();

    // Draw the game state
    void renderGameState(const GameState &state, float interpolation = 1.0f);

    // Update display of chat messages
    void renderChatMessages(const std::vector<ChatMessageData> &messages);

    // Draw UI elements
    void renderScore(const GameState &state);
    void renderControls();

    // Handle chat input display
    void renderChatInput(const std::string &inputText);

    // Initialize chat area
    void initializeChatArea(const std::string &opponentName);

    // Render a goal scored animation
    void renderGoalAnimation();

  private:
    // Previous state for interpolation
    GameState prevState;

    // Last rendered ball position for erasing
    int lastBallX, lastBallY;

    // Game area dimensions
    int width, height;

    // Functions to draw game elements
    void drawArena();
    void drawPaddle(const Paddle &paddle, bool erase = false);
    void drawBall(const Ball &ball, bool erase = false);
    void drawChatArea();

    // String formatting helpers
    std::string getColoredText(const std::string &text, int color) const;

    // Interpolate between two states for smooth rendering
    GameState interpolateStates(const GameState &prev, const GameState &current, float alpha);
};

} // namespace pong