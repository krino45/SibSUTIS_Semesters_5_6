// client/input.cpp
#include "input.h"
#include "../common/utils.h"
#include <cstring>
#include <fstream>
#include <iostream>

namespace pong
{
InputHandler *InputHandler::globalInputHandler = nullptr;

void restoreTerminal()
{
    if (InputHandler::globalInputHandler)
    {
        InputHandler::globalInputHandler->disableRawMode();
        pong::terminal::showCursor();
        pong::terminal::resetColor();
        std::cout << std::endl;
    }
}

InputHandler::InputHandler() : rawModeEnabled(false), chatMode(false), forcedQuit(false)
{
    // Store the original terminal settings
    tcgetattr(STDIN_FILENO, &originalTermios);
    globalInputHandler = this;
}

InputHandler::~InputHandler()
{
    tcsetattr(STDIN_FILENO, 0, &originalTermios);
    pong::terminal::showCursor();
    pong::terminal::resetColor();
}

void InputHandler::prepareForMenuInput()
{
    if (chatMode)
        stopChatMode();

    disableRawMode(); // Canonical + echo
    terminal::showCursor();

    // Flush any pending input
    tcflush(STDIN_FILENO, TCIFLUSH);

    // Ensure stdin is reopened if it was closed
    if (std::cin.eof() || std::cin.fail())
    {
        // std::cout << "reopening stdin\n ";
        static std::ifstream tty("/dev/tty");
        std::cin.rdbuf(tty.rdbuf());
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }
    std::cin.clear();

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    char discard;
    // int flushed = 0;

    while (read(STDIN_FILENO, &discard, 1) > 0)
    {
        // flushed++;
    }
    // std::cout << "[DEBUG] Flushed " << flushed << " bytes from stdin\n";

    fcntl(STDIN_FILENO, F_SETFL, flags); // Reset flags
}

bool InputHandler::initialize()
{
    enableRawMode();
    return rawModeEnabled;
}

void InputHandler::enableRawMode()
{
    // Get current terminal settings
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);

    // Turn off echoing and canonical mode
    raw.c_lflag &= ~(ECHO | ICANON);

    // Set minimum chars and timeout
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (fcntl(STDIN_FILENO, F_GETFL) == -1)
    {
        std::cerr << "[WARN] STDIN_FILENO invalid @ enableRawMode, skipping terminal ops" << std::endl;
        return;
    }
    // Apply settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    // Set stdin to non-blocking mode
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    rawModeEnabled = true;
}

void InputHandler::disableRawMode()
{
    // Restore original terminal settings
    if (fcntl(STDIN_FILENO, F_GETFL) == -1)
    {
        std::cerr << "[WARN] STDIN_FILENO invalid @ disableRawMode, skipping terminal ops" << std::endl;
        return;
    }
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTermios);

    // Reset non-blocking mode
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);

    rawModeEnabled = false;
}

bool InputHandler::hasInput()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    // Zero timeout for polling
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(1, &readfds, NULL, NULL, &timeout) > 0;
}

char InputHandler::readChar()
{
    char c = 0;
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
    {
        perror("read");
    }
    return c;
}

uint8_t InputHandler::poll()
{
    if (forcedQuit)
    {
        forcedQuit = false;
        return 0 | InputFlags::QUIT;
    }

    if (chatMode)
        return InputFlags::NONE;

    uint8_t input = InputFlags::NONE;

    while (hasInput())
    {
        char c = readChar();

        // Handle quit
        if (c == 'q' || c == 'Q')
        {
            input |= InputFlags::QUIT;
            if (quitCallback)
                quitCallback();
        }

        // Handle paddle movement
        if (c == 'w' || c == 'W')
            input |= InputFlags::UP;
        if (c == 's' || c == 'S')
            input |= InputFlags::DOWN;

        // Handle arrow keys (3-byte sequences)
        if (c == '\033')
        {
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) != -1 && read(STDIN_FILENO, &seq[1], 1) != -1)
            {
                if (seq[0] == '[')
                {
                    if (seq[1] == 'A')
                        input |= InputFlags::ARROW_UP; // Up arrow
                    if (seq[1] == 'B')
                        input |= InputFlags::ARROW_DOWN; // Down arrow
                }
            }
        }

        // Handle chat activation
        if (c == 't' || c == 'T')
        {
            if (chatCallback)
                chatCallback();
        }
    }

    return input;
}

void InputHandler::startChatMode()
{
    chatMode = true;
    currentChatInput = "";

    // Switch to blocking mode for chat
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

void InputHandler::stopChatMode()
{
    chatMode = false;

    // Switch back to non-blocking mode
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

std::string InputHandler::getChatInput()
{
    if (!chatMode)
        return "";

    // Save terminal settings
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // Enable echo and canonical mode for chat input
    disableRawMode();
    terminal::showCursor();
    newt.c_lflag |= (ECHO | ICANON);

    if (fcntl(STDIN_FILENO, F_GETFL) == -1)
    {
        std::cerr << "[WARN] STDIN_FILENO invalid @ getChatInput, skipping terminal ops" << std::endl;
        return "";
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    // Get the input
    std::string input;
    std::getline(std::cin, input);

    if (fcntl(STDIN_FILENO, F_GETFL) == -1)
    {
        std::cerr << "[WARN] STDIN_FILENO invalid @ getChatInput, skipping terminal ops" << std::endl;
        return "";
    }
    // Restore raw mode settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    // Return the input
    return input;
}

} // namespace pong