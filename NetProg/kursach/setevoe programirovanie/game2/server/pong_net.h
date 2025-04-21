#ifndef PONG_NET_H
#define PONG_NET_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <vector>
#include <map>

enum GameMode { LOCAL, ONLINE };

class NetworkManager {
public:
    bool connectToServer(const std::string& serverIp, int port, 
                        const std::string& username, int mmr, 
                        int udp_port, int tcp_port);
    void setupUDP(const std::string& ip, int port);
    void setupTCP(const std::string& ip, int port);
    void sendGameState(float ball_x, float ball_y, float ball_dx, float ball_dy, 
                      int p1_y, int p2_y, int p1_score, int p2_score);
                      
    bool receiveGameState(float& ball_x, float& ball_y, float& ball_dx, float& ball_dy,
                        int& p1_y, int& p2_y, int& p1_score, int& p2_score);
    void sendChatMessage(const std::string& message);
    bool receiveChatMessage(std::string& message);
    std::string getOpponentUsername() const { return opponent_username; }

private:
    int sock = -1;
    int udp_sock = -1;
    int tcp_sock;
    sockaddr_in opponent_tcp_addr;
    sockaddr_in opponent_addr;
    std::string opponent_username;
    int opponent_mmr;
    std::string opponent_ip;
    int opponent_udp_port;
    int opponent_tcp_port;
    int packets_sent = 0;
    int packets_received = 0;
    std::chrono::steady_clock::time_point last_debug_time = std::chrono::steady_clock::now();
};

#endif