// server / pong_net.cpp
#include "../server/pong_net.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

bool NetworkManager::connectToServer(const std::string& serverIp, int port, 
                                    const std::string& username, int mmr, 
                                    int udp_port, int tcp_port) {
    // Create TCP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return false;
    }
    
    // Configure server address
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, serverIp.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported" << std::endl;
        close(sock);
        sock = -1;
        return false;
    }
    
    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        close(sock);
        sock = -1;
        return false;
    }
    
    // Prepare request message: username:mmr:udp_port:tcp_port
    std::string request = username + ":" + std::to_string(mmr) + ":" + 
                         std::to_string(udp_port) + ":" + std::to_string(tcp_port);
    
    // Send request to server
    send(sock, request.c_str(), request.length(), 0);
    
    // Wait for response
    char buffer[1024] = {0};
    int bytes_read = read(sock, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        std::cerr << "Failed to read from server" << std::endl;
        close(sock);
        sock = -1;
        return false;
    }
    
    // Parse response
    std::string response(buffer);
    
    if (response == "WAITING") {
        std::cout << "Waiting for an opponent..." << std::endl;
        
        // Wait for match notification
        memset(buffer, 0, sizeof(buffer));
        bytes_read = read(sock, buffer, sizeof(buffer));
        if (bytes_read <= 0) {
            std::cerr << "Lost connection while waiting for match" << std::endl;
            close(sock);
            sock = -1;
            return false;
        }
        
        response = std::string(buffer);
    }
    
    // Check if we got matched
    if (response.substr(0, 8) != "MATCHED:") {
        std::cerr << "Unexpected server response: " << response << std::endl;
        close(sock);
        sock = -1;
        return false;
    }
    
    // Parse match info: MATCHED:username:mmr:ip:udp_port
    std::string match_info = response.substr(8);
    size_t colon1 = match_info.find(':');
    size_t colon2 = match_info.find(':', colon1+1);
    size_t colon3 = match_info.find(':', colon2+1);
    size_t colon4 = match_info.find(':', colon3+1);  // Add this line to find the new delimiter

    if (colon1 == std::string::npos || colon2 == std::string::npos || 
        colon3 == std::string::npos || colon4 == std::string::npos) {
        std::cerr << "Invalid match info: " << match_info << std::endl;
        close(sock);
        sock = -1;
        return false;
    }

    opponent_username = match_info.substr(0, colon1);
    opponent_mmr = std::stoi(match_info.substr(colon1+1, colon2-colon1-1));
    opponent_ip = match_info.substr(colon2+1, colon3-colon2-1);
    opponent_udp_port = std::stoi(match_info.substr(colon3+1, colon4-colon3-1));
    opponent_tcp_port = std::stoi(match_info.substr(colon4+1));
    
    // Set up UDP socket for game state updates
    setupUDP(serverIp, udp_port);
    setupTCP(opponent_ip, opponent_tcp_port);
    
    return true;
}

void NetworkManager::setupUDP(const std::string& ip, int port) {
    // Create UDP socket
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        std::cerr << "Failed to create UDP socket" << std::endl;
        return;
    }
    
    // Set non-blocking mode
    int flags = fcntl(udp_sock, F_GETFL, 0);
    fcntl(udp_sock, F_SETFL, flags | O_NONBLOCK);
    
    // Bind to local port
    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(port);
    
    if (bind(udp_sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        std::cerr << "UDP bind failed" << std::endl;
        close(udp_sock);
        udp_sock = -1;
        return;
    }
    
    // Setup opponent address for sending
    opponent_addr.sin_family = AF_INET;
    opponent_addr.sin_port = htons(opponent_udp_port);
    inet_pton(AF_INET, opponent_ip.c_str(), &opponent_addr.sin_addr);
}

void NetworkManager::setupTCP(const std::string& ip, int port) {
    // Create TCP socket for chat
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sock < 0) {
        std::cerr << "Failed to create TCP socket for chat" << std::endl;
        return;
    }
    
    // Set non-blocking mode
    int flags = fcntl(tcp_sock, F_GETFL, 0);
    fcntl(tcp_sock, F_SETFL, flags | O_NONBLOCK);
    
    // Connect to opponent
    opponent_tcp_addr.sin_family = AF_INET;
    opponent_tcp_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &opponent_tcp_addr.sin_addr);
    
    connect(tcp_sock, (struct sockaddr*)&opponent_tcp_addr, sizeof(opponent_tcp_addr));
}


void NetworkManager::sendGameState(float ball_x, float ball_y, float ball_dx, float ball_dy,
                                 int p1_y, int p2_y, int p1_score, int p2_score) {
    if (udp_sock < 0) return;
    
    // Format: GAME:ball_x:ball_y:ball_dx:ball_dy:p1_y:p2_y:p1_score:p2_score
    std::string data = "GAME:" + std::to_string(ball_x) + ":" +
                      std::to_string(ball_y) + ":" +
                      std::to_string(ball_dx) + ":" +
                      std::to_string(ball_dy) + ":" +
                      std::to_string(p1_y) + ":" +
                      std::to_string(p2_y) + ":" +
                      std::to_string(p1_score) + ":" +
                      std::to_string(p2_score);
    
    int result = sendto(udp_sock, data.c_str(), data.length(), 0,
          (struct sockaddr*)&opponent_addr, sizeof(opponent_addr));
          
    if (result > 0) packets_sent++;
    
    // Print debug stats every 5 seconds
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_debug_time).count() >= 5) {
        std::cout << "Network stats: Sent " << packets_sent << " packets, Received " 
                  << packets_received << " packets" << std::endl;
        packets_sent = 0;
        packets_received = 0;
        last_debug_time = now;
    }
}

// Modify the receiveGameState method to be smarter about what it updates
bool NetworkManager::receiveGameState(float& ball_x, float& ball_y, float& ball_dx, float& ball_dy,
                                    int& p1_y, int& p2_y, int& p1_score, int& p2_score) {
    if (udp_sock < 0) return false;
    
    char buffer[1024] = {0};
    sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    
    int bytes_received = recvfrom(udp_sock, buffer, sizeof(buffer), 0,
                                (struct sockaddr*)&sender_addr, &addr_len);
    
    if (bytes_received <= 0) return false;
    packets_received++;
    
    std::string data(buffer);
    if (data.substr(0, 5) != "GAME:") return false;
    
    data = data.substr(5); // Remove "GAME:" prefix
    
    // Parse all components
    std::vector<std::string> components;
    size_t pos = 0;
    while ((pos = data.find(':')) != std::string::npos) {
        components.push_back(data.substr(0, pos));
        data.erase(0, pos + 1);
    }
    components.push_back(data); // Add last component
    
    if (components.size() != 8) return false;
    
    try {
        // Always update both paddles
        p1_y = std::stoi(components[4]);
        p2_y = std::stoi(components[5]);
        
        // Only player 1 is authoritative about ball and score
        if (components[0] != "0") { // Check if ball data is included
            ball_x = std::stof(components[0]);
            ball_y = std::stof(components[1]);
            ball_dx = std::stof(components[2]);
            ball_dy = std::stof(components[3]);
            p1_score = std::stoi(components[6]);
            p2_score = std::stoi(components[7]);
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

void NetworkManager::sendChatMessage(const std::string& message) {
    if (udp_sock < 0) return;
    
    std::string data = "CHAT:" + message;
    
    send(tcp_sock, data.c_str(), data.length(), 0);
}

// Add a method to receive chat messages
bool NetworkManager::receiveChatMessage(std::string& message) {
    if (tcp_sock < 0) return false;
    
    char buffer[1024] = {0};
    int bytes_received = recv(tcp_sock, buffer, sizeof(buffer), 0);
    
    if (bytes_received <= 0) return false;
    
    std::string data(buffer);
    if (data.substr(0, 5) != "CHAT:") return false;
    
    message = data.substr(5);
    return true;
}