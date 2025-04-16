// common/utils.h
#pragma once

#include <chrono>
#include <thread>
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <condition_variable>
#include <iostream>

namespace pong {

// Terminal control utilities
namespace terminal {
    inline void clearScreen() {
        std::cout << "\033[2J\033[H";
    }

    inline void setCursor(int x, int y) {
        std::cout << "\033[" << y << ";" << x << "H";
    }

    inline void hideCursor() {
        std::cout << "\033[?25l";
    }

    inline void showCursor() {
        std::cout << "\033[?25h";
    }

    inline void resetColor() {
        std::cout << "\033[0m";
    }

    inline std::string colorText(const std::string& text, int fg, int bg = -1) {
        std::stringstream ss;
        ss << "\033[38;5;" << fg << "m";
        if (bg >= 0) {
            ss << "\033[48;5;" << bg << "m";
        }
        ss << text << "\033[0m";
        return ss.str();
    }
}

// Time-related utilities
class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    Timer() : startTime(Clock::now()) {}

    void reset() {
        startTime = Clock::now();
    }

    double elapsedMilliseconds() const {
        auto now = Clock::now();
        return std::chrono::duration<double, std::milli>(now - startTime).count();
    }

    double elapsedSeconds() const {
        return elapsedMilliseconds() / 1000.0;
    }

private:
    TimePoint startTime;
};

// Thread-safe queue for message passing between threads
template<typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    void push(T item) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(std::move(item));
        cv.notify_one();
    }

    bool tryPop(T& item) {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty()) {
            return false;
        }
        item = std::move(queue.front());
        queue.pop();
        return true;
    }

    T pop() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !queue.empty(); });
        T item = std::move(queue.front());
        queue.pop();
        return item;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }

private:
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable cv;
};

// Formatted timestamp string
inline std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&tt), "%H:%M:%S");
    return ss.str();
}

// Safe sleep that handles interrupts
inline void safeSleep(std::chrono::microseconds duration) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + duration;
    
    while (std::chrono::high_resolution_clock::now() < end) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

} // namespace pong