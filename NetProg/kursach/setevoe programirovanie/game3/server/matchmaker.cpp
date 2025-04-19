// server/matchmaker.cpp

#include "matchmaker.h"
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

namespace pong
{

Matchmaker::Matchmaker()
{
}

Matchmaker::~Matchmaker()
{
}

uint8_t Matchmaker::addPlayer(const PlayerInfo &player)
{
    std::lock_guard<std::mutex> lock(mutex);
    waitingPlayers.push(player);
    cv.notify_one();
    return waitingPlayers.size();
}

bool Matchmaker::findMatch(PlayerInfo &player1, PlayerInfo &player2)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (waitingPlayers.size() >= 2)
    {
        player2 = waitingPlayers.front();
        waitingPlayers.pop();
        player1 = waitingPlayers.front();
        waitingPlayers.pop();

        matchedPlayers[player1.username] = player2;
        matchedPlayers[player2.username] = player1;
        return true;
    }
    return false;
}

void Matchmaker::process()
{
    PlayerInfo player1, player2;
    if (findMatch(player1, player2))
    {
        std::cout << "Match found: " << player1.username << " vs " << player2.username << std::endl;
        notifyPlayerAboutMatch(player1, player2);
        notifyPlayerAboutMatch(player2, player1);
    }
}

void Matchmaker::notifyPlayerAboutMatch(const PlayerInfo &player, const PlayerInfo &opponent)
{
    // Create a match notification packet
    std::vector<uint8_t> packet = createMatchNotificationPacket(player, opponent);

    // Send the packet to the player
    sendPacketToPlayer(player, packet);
    std::cout << "Sent notification packet (" << packet.data() << ") to player "
              << player.username + "@" + player.address + ":" << player.udpPort << std::endl;
}

std::vector<uint8_t> Matchmaker::createMatchNotificationPacket(const PlayerInfo &player, const PlayerInfo &opponent)
{
    // Create a ConnectResponse structure to notify the player about the match
    ConnectResponse response;
    response.success = true;
    strncpy(response.opponentName, opponent.username.c_str(), sizeof(response.opponentName));
    strncpy(response.hostAddress, opponent.address.c_str(), sizeof(response.hostAddress));
    response.hostUdpPort = opponent.udpPort;
    response.hostTcpPort = opponent.tcpPort;
    response.isPlayer1 = (player.username < opponent.username); // Arbitrary decision for player order

    // Serialize the response into a packet
    return createPacket(MessageType::CONNECT_RESPONSE, 0, &response, sizeof(response));
}

void Matchmaker::sendPacketToPlayer(const PlayerInfo &player, const std::vector<uint8_t> &packet)
{
    // Create a UDP socket
    int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1)
    {
        perror("socket");
        return;
    }

    // Set up the address structure
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(player.udpPort);
    inet_pton(AF_INET, player.address.c_str(), &addr.sin_addr);

    // Send the packet to the player
    sendto(udpSocket, packet.data(), packet.size(), 0, (struct sockaddr *)&addr, sizeof(addr));

    // Close the socket
    close(udpSocket);
}

} // namespace pong
