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

    bool initialize();

    void clearScreen();

    void renderGameState(const GameState &state, float interpolation = 1.0f);

    void renderChatMessages(const std::vector<ChatMessageData> &messages);

    void renderScore(const GameState &state);
    void renderControls();

    void renderChatInput(const std::string &inputText);

    void initializeChatArea(const std::string &opponentName);

    void renderGoalAnimation();
    void showMatchFoundAnimation(const std::string &opponentName);
    void showVictoryScreen(const std::string &winnerName, int player1Score, int player2Score);
    void showDisconnectMessage();

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