// common/game_state.h
#pragma once

#include <cmath>
#include <cstdint>
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
    static constexpr float WIDTH = 88.0f;
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
        ball.velocity = {serve_left ? -ball.speed : ball.speed, ((rand() % 100) / 100.0f - 0.5f) * 0.5f};

        player1.position = {2.0f, HEIGHT / 2 - player1.size.y / 2};
        player2.position = {WIDTH - 2 - player2.size.x, HEIGHT / 2 - player2.size.y / 2};
    }
    void update()
    {
        ball.last_position = ball.position;
        ball.position = ball.position + ball.velocity;

        // Wall collision
        if (ball.position.y <= 1.0f || ball.position.y >= HEIGHT - 1.0f)
        {
            ball.position.y = ball.position.y <= 1.0f ? 1.0f : HEIGHT - 1.0f;
            ball.velocity.y *= -0.95f;
        }

        // Paddle collision
        auto checkPaddleCollision = [this](const Paddle &paddle) {
            return ball.position.x >= paddle.position.x - 1.0f &&
                   ball.position.x <= paddle.position.x + paddle.size.x + 1.0f &&
                   ball.position.y >= paddle.position.y - 0.5f &&
                   ball.position.y <= paddle.position.y + paddle.size.y + 0.5f;
        };

        if (checkPaddleCollision(player1))
        {
            ball.speed = std::min(ball.speed + BALL_SPEED_INCREASE, BALL_BASE_SPEED * 2.0f);
            float hit_pos = (ball.position.y - player1.position.y) / player1.size.y - 0.5f;
            ball.velocity = {fabs(ball.velocity.x), hit_pos * 1.3f};
            ball.position.x = player1.position.x + player1.size.x + 0.1f;
        }
        else if (checkPaddleCollision(player2))
        {
            ball.speed = std::min(ball.speed + BALL_SPEED_INCREASE, BALL_BASE_SPEED * 2.0f);
            float hit_pos = (ball.position.y - player2.position.y) / player2.size.y - 0.5f;
            ball.velocity = {-fabs(ball.velocity.x), hit_pos * 1.3f};
            ball.position.x = player2.position.x - 0.1f;
        }

        // Normalize velocity
        float mag = std::sqrt(ball.velocity.x * ball.velocity.x + ball.velocity.y * ball.velocity.y);
        if (mag > 0)
        {
            ball.velocity = (ball.velocity * (1.0f / mag)) * ball.speed;
        }

        // Scoring
        if (ball.position.x < 0 || ball.position.x >= WIDTH)
        {
            if (ball.position.x < 0)
                player2.score++;
            else
                player1.score++;

            reset(ball.position.x < 0);
        }

        frame++;
    }

    void serialize(std::vector<uint8_t> &buffer) const
    {
        buffer.resize(sizeof(GameState));
        memcpy(buffer.data(), this, sizeof(GameState));
    }

    bool deserialize(const std::vector<uint8_t> &buffer)
    {
        if (buffer.size() != sizeof(GameState))
            return false;
        memcpy(this, buffer.data(), sizeof(GameState));
        return true;
    }
};