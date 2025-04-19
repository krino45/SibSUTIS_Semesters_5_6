// common/network.h
#pragma once

#include "game_state.h"
#include <cstdint>
#include <string>
#include <vector>

namespace pong
{

// Message types for protocol
enum class MessageType : uint8_t
{
    // Control messages
    CONNECT_REQUEST = 0,
    CONNECT_RESPONSE,
    DISCONNECT,

    // Game state messages
    GAME_STATE_UPDATE,
    PLAYER_INPUT,

    // Chat messages
    CHAT_MESSAGE,

    // Matchmaking messages
    MATCHMAKING_REQUEST,
    MATCHMAKING_RESPONSE
};

// Input flags
enum InputFlags : uint8_t
{
    NONE = 0x00,
    UP = 0x01,
    DOWN = 0x02,
    QUIT = 0x04,
    ARROW_UP = 0x08,
    ARROW_DOWN = 0x10,
};

// Network header for all messages
struct NetworkHeader
{
    MessageType type;
    uint32_t frame;
    uint32_t dataSize;
};

// All message structs
struct ConnectRequest
{
    char username[32];
    uint16_t udpPort;
    uint16_t tcpPort;
    uint32_t mmr;
};

struct ConnectResponse
{
    bool success;
    char opponentName[32];
    char hostAddress[16]; // IPv4 address string
    uint16_t hostUdpPort;
    uint16_t hostTcpPort;
    bool isPlayer1;
};

struct PlayerInput
{
    uint8_t playerId;
    uint8_t flags; // Combination of InputFlags
    uint32_t frameNumber;
};

struct ChatMessageData
{
    char sender[32];
    uint32_t timestamp;
    uint16_t contentLength;
    char content[1024];
};

// Constants for network communications
constexpr int MAX_PACKET_SIZE = 1024;
constexpr int MAX_CHAT_SIZE = 512;
constexpr int UDP_SERVER_PORT = 8080;
constexpr int TCP_SERVER_PORT = 8081;
constexpr int HEADER_SIZE = sizeof(NetworkHeader);

// UDP packet serialization/deserialization functions
std::vector<uint8_t> createPacket(MessageType type, uint32_t frame, const void *data, uint32_t dataSize);
std::vector<uint8_t> createInputPacket(uint8_t inputFlags, uint32_t frame);
std::vector<uint8_t> createChatPacket(const std::string &sender, const std::string &message);
std::vector<uint8_t> createInputPacket(const PlayerInput &input);

// Helper to parse a network header from a buffer
NetworkHeader parseHeader(const std::vector<uint8_t> &packet);

} // namespace pong