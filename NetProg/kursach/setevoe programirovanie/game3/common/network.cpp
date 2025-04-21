// common/network.cpp
#include "network.h"
#include <chrono>
#include <cstring>

namespace pong
{

std::vector<uint8_t> createPacket(MessageType type, uint32_t frame, const void *data, uint32_t dataSize)
{
    NetworkHeader header;
    memset(&header, 0, sizeof(header));
    header.type = type;
    header.frame = frame;
    header.dataSize = dataSize;

    std::vector<uint8_t> packet(sizeof(header) + dataSize, 0);
    memcpy(packet.data(), &header, sizeof(header));

    if (data && dataSize > 0)
    {
        memcpy(packet.data() + sizeof(header), data, dataSize);
    }

    return packet;
}

std::vector<uint8_t> createInputPacket(uint8_t inputFlags, uint8_t frame)
{
    PlayerInput input{inputFlags, frame};
    return createPacket(MessageType::PLAYER_INPUT, frame, &input, sizeof(PlayerInput));
}

std::vector<uint8_t> createChatPacket(const std::string &sender, const std::string &message)
{
    ChatMessageData chatMsg{};
    strncpy(chatMsg.sender, sender.c_str(), sizeof(chatMsg.sender) - 1);
    chatMsg.timestamp = static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    chatMsg.contentLength = std::min<uint16_t>(message.size(), sizeof(chatMsg.content) - 1);
    strncpy(chatMsg.content, message.c_str(), chatMsg.contentLength);
    chatMsg.content[chatMsg.contentLength] = '\0';

    return createPacket(MessageType::CHAT_MESSAGE, 0, &chatMsg, sizeof(ChatMessageData));
}

std::vector<uint8_t> createInputPacket(const PlayerInput &input)
{
    // Ensure padding bytes are initialized
    PlayerInput cleanedInput;
    memset(&cleanedInput, 0, sizeof(cleanedInput));
    cleanedInput.playerId = input.playerId;
    cleanedInput.flags = input.flags;
    cleanedInput.frameNumber = input.frameNumber;

    return createPacket(MessageType::PLAYER_INPUT, input.frameNumber, &cleanedInput, sizeof(cleanedInput));
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