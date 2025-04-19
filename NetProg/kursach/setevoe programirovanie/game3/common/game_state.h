// common/game_state.h
#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>

struct Vec2
{
    double x, y;

    double magnitude() const
    {
        return sqrt(x * x + y * y);
    }

    Vec2 operator+(const Vec2 &other) const
    {
        Vec2 result;
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
    }

    Vec2 operator-(const Vec2 &other) const
    {
        Vec2 result;
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
    }

    Vec2 operator*(const Vec2 &other) const
    {
        Vec2 result;
        result.x = x * other.x;
        result.y = y * other.y;
        return result;
    }

    Vec2 operator*(float scalar) const
    {
        Vec2 result;
        result.x = x * scalar;
        result.y = y * scalar;
        return result;
    }

    static Vec2 lerp(const Vec2 &a, const Vec2 &b, float t)
    {
        return a + (b - a) * t;
    }
};

struct Paddle
{
    static constexpr float HEIGHT = 8.0f;
    static constexpr float WIDTH = 1.0f;

    Vec2 position;
    Vec2 size;
    int32_t score;
    uint32_t color;
};

struct Ball
{
    static constexpr float RADIUS = 1.0f;

    Vec2 position;
    Vec2 last_position;
    Vec2 velocity;
    float speed;
};

struct GameState
{
    static constexpr float WIDTH = 84.0f;
    static constexpr float HEIGHT = 24.0f;
    static constexpr float PADDLE_SPEED = 1.5f;
    static constexpr float BALL_BASE_SPEED = 0.2f;
    static constexpr float BALL_SPEED_INCREASE = 0.05f;

    Paddle player1;
    Paddle player2;
    Ball ball;
    uint32_t frame;

    void reset(bool serve_left)
    {
        ball.position = {WIDTH / 2.0f, HEIGHT / 2.0f};
        ball.speed = BALL_BASE_SPEED;
        float angle = ((std::rand() % 100) / 100.0f - 0.5f) * (3.14159f / 2);
        ball.velocity.x = ((serve_left) ? -1.0f : 1.0f) * BALL_BASE_SPEED * std::cos(angle);
        ball.velocity.y = BALL_BASE_SPEED * std::sin(angle);

        player1.position = {2.0f, HEIGHT / 2 - player1.size.y / 2};
        player2.position = {WIDTH - 2 - player2.size.x, HEIGHT / 2 - player2.size.y / 2};
    }
    void update()
    {
        // Update ball position
        ball.position = ball.position + ball.velocity;

        // Check collision with walls
        if (ball.position.y <= 1 || ball.position.y >= HEIGHT - 2)
        {
            ball.velocity.y = -ball.velocity.y;
            if (ball.position.y <= 1)
            {
                ball.position.y = 1;
            }
            else
            {
                ball.position.y = HEIGHT - 2;
            }
        }

        // Check for collision with paddles
        if (ball.position.x <= player1.position.x + Paddle::WIDTH && ball.position.x >= player1.position.x &&
            ball.position.y >= player1.position.y && ball.position.y <= player1.position.y + Paddle::HEIGHT)
        {

            // Calculate reflection angle based on where the ball hit the paddle
            float relativeIntersectY = (player1.position.y + (Paddle::HEIGHT / 2)) - ball.position.y;
            float normalizedRelativeIntersectionY = (relativeIntersectY / (Paddle::HEIGHT / 2));
            float bounceAngle = normalizedRelativeIntersectionY * (3.14159f / 4); // Max 45 degrees

            ball.velocity.x = std::abs(ball.velocity.x); // Force direction away from paddle
            ball.velocity.y = -std::sin(bounceAngle) * ball.velocity.x;

            // Add a small speed increase on each hit
            const float speedIncrease = 0.1f;
            float currentSpeed = std::sqrt(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y);
            float ratio = (currentSpeed + speedIncrease) / currentSpeed;

            ball.velocity.x *= ratio;
            ball.velocity.y *= ratio;
        }

        if (ball.position.x >= player2.position.x - Ball::RADIUS && ball.position.x <= player2.position.x &&
            ball.position.y >= player2.position.y && ball.position.y <= player2.position.y + Paddle::HEIGHT)
        {

            // Calculate reflection angle based on where the ball hit the paddle
            float relativeIntersectY = (player2.position.y + (Paddle::HEIGHT / 2)) - ball.position.y;
            float normalizedRelativeIntersectionY = (relativeIntersectY / (Paddle::HEIGHT / 2));
            float bounceAngle = normalizedRelativeIntersectionY * (3.14159f / 4); // Max 45 degrees

            ball.velocity.x = -std::abs(ball.velocity.x); // Force direction away from paddle
            ball.velocity.y = -std::sin(bounceAngle) * std::abs(ball.velocity.x);

            // Add a small speed increase on each hit
            const float speedIncrease = 0.1f;
            float currentSpeed = std::sqrt(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y);
            float ratio = (currentSpeed + speedIncrease) / currentSpeed;

            ball.velocity.x *= ratio;
            ball.velocity.y *= ratio;
        }

        // Check for scoring
        if (ball.position.x <= 0)
        {
            player2.score++;
            reset(true);
        }

        if (ball.position.x >= WIDTH - 1)
        {
            player1.score++;
            reset(false);
        }

        frame++;
    }

    bool deserialize(const std::vector<uint8_t> &const_buffer)
    {
        std::vector<uint8_t> buffer = const_buffer;
        buffer.shrink_to_fit();
        if (buffer.size() != sizeof(GameState))
        {
            std::cerr << "Deserialize failed: buffer size " << buffer.size() << " != expected " << sizeof(GameState)
                      << std::endl;
            return false;
        }
        memcpy(this, buffer.data(), sizeof(GameState));
        return true;
    }
};