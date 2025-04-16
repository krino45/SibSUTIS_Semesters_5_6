// client/input.h
#pragma once

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <queue>
#include <functional>
#include "../common/network.h"

namespace pong {

// Input handler class
class InputHandler {
public:
    InputHandler();
    ~InputHandler();
    
    // Initialize terminal for raw input
    bool initialize();
    
    // Process any waiting input
    uint8_t poll();
    
    // Enable/disable raw mode
    void enableRawMode();
    void disableRawMode();
    
    // Check if we have input available
    bool hasInput();
    
    // Callbacks
    void setQuitCallback(std::function<void()> callback) { quitCallback = callback; }
    void setChatCallback(std::function<void()> callback) { chatCallback = callback; }
    
    // For chat input
    void startChatMode();
    void stopChatMode();
    bool isInChatMode() { return chatMode; }
    std::string getChatInput();

private:
    struct termios originalTermios;
    bool rawModeEnabled;
    bool chatMode;
    std::string currentChatInput;
    
    std::function<void()> quitCallback;
    std::function<void()> chatCallback;
    
    char readChar();
};

} // namespace pong