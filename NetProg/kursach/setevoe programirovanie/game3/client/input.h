// client/input.h
#pragma once

#include "../common/network.h"
#include <fcntl.h>
#include <functional>
#include <mutex>
#include <queue>
#include <termios.h>
#include <unistd.h>

namespace pong
{

void restoreTerminal();

// Input handler class
class InputHandler
{
  public:
    static InputHandler *globalInputHandler;

    InputHandler();
    ~InputHandler();

    void prepareForMenuInput();

    // Initialize terminal for raw input
    bool initialize();

    // Process any waiting input
    uint8_t poll();

    void forceQuit()
    {
        std::lock_guard<std::mutex> lock(forcedInputMutex);
        forcedQuit = true;
    }

    // Enable/disable raw mode
    void enableRawMode();
    void disableRawMode();

    // Check if we have input available
    bool hasInput();

    // Callbacks
    void setQuitCallback(std::function<void()> callback)
    {
        quitCallback = callback;
    }
    void setChatCallback(std::function<void()> callback)
    {
        chatCallback = callback;
    }

    // For chat input
    void startChatMode();
    void stopChatMode();
    bool isInChatMode()
    {
        return chatMode;
    }
    std::string getChatInput();

  private:
    struct termios originalTermios;
    bool rawModeEnabled;
    bool chatMode;
    std::string currentChatInput;

    std::mutex forcedInputMutex;
    bool forcedQuit;

    std::function<void()> quitCallback;
    std::function<void()> chatCallback;

    char readChar();
};

} // namespace pong