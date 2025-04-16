// common/network.cpp
#include "network.h"
#include <chrono>
#include <cstring>

namespace pong
{

std::vector<uint8_t> createPacket(MessageType type, uint32_t frame, const void *data, uint32_t dataSize)
{
    std::vector<uint8_t> packet(HEADER_SIZE + dataSize);

    // Create the header
    NetworkHeader header{type, frame, dataSize};

    // Copy the header to the packet
    memcpy(packet.data(), &header, HEADER_SIZE);

    // Copy the data (if any) to the packet
    if (data != nullptr && dataSize > 0)
    {
        memcpy(packet.data() + HEADER_SIZE, data, dataSize);
    }

    return packet;
}

std::vector<uint8_t> createGameStatePacket(const GameState &state)
{
    std::vector<uint8_t> stateData;
    state.serialize(stateData);
    return createPacket(MessageType::GAME_STATE_UPDATE, state.frame, stateData.data(), stateData.size());
}

std::vector<uint8_t> createInputPacket(uint8_t inputFlags, uint32_t frame)
{
    PlayerInput input{inputFlags, frame};
    return createPacket(MessageType::PLAYER_INPUT, frame, &input, sizeof(PlayerInput));
}

std::vector<uint8_t> createChatPacket(const std::string &sender, const std::string &message)
{
    // Calculate total size needed for chat message
    uint16_t contentLength = static_cast<uint16_t>(message.size());
    uint32_t totalSize = sizeof(ChatMessageData) + contentLength;

    // Create the buffer for the chat message
    std::vector<uint8_t> chatData(totalSize);
    ChatMessageData *chatMsg = reinterpret_cast<ChatMessageData *>(chatData.data());

    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    uint32_t timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

    // Fill chat message header
    strncpy(chatMsg->sender, sender.c_str(), sizeof(chatMsg->sender) - 1);
    chatMsg->sender[sizeof(chatMsg->sender) - 1] = '\0'; // Ensure null termination
    chatMsg->timestamp = timestamp;
    chatMsg->contentLength = contentLength;

    // Copy message content
    if (contentLength > 0)
    {
        memcpy(chatData.data() + sizeof(ChatMessageData), message.c_str(), contentLength);
    }

    // Create the complete packet
    return createPacket(MessageType::CHAT_MESSAGE, 0, chatData.data(), totalSize);
}

NetworkHeader parseHeader(const std::vector<uint8_t> &packet)
{
    NetworkHeader header;
    if (packet.size() >= HEADER_SIZE)
    {
        memcpy(&header, packet.data(), HEADER_SIZE);
    }
    return header;
}

} // namespace pong