// server / pong_server.cpp
#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <condition_variable>
#include <chrono>

const int PORT = 8080;

struct Player {
    std::string username;
    int mmr;
    sockaddr_in address;
    int udp_port;
    int tcp_port;
    int client_socket;
};

struct PendingMatch {
    Player player1;
    Player player2;
    bool notification_sent;
};

std::vector<Player> waiting_players;
std::map<std::string, PendingMatch> pending_matches;
std::mutex players_mutex;
std::condition_variable match_cv;

// Function to set socket to non-blocking mode
void set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

// Function to send match notification to a player
bool send_match_notification(const Player& player, const Player& opponent) {
std::string response = "MATCHED:" + opponent.username + ":" + 
                     std::to_string(opponent.mmr) + ":" + 
                     inet_ntoa(opponent.address.sin_addr) + ":" + 
                     std::to_string(opponent.udp_port) + ":" +
                     std::to_string(opponent.tcp_port);
    
    int bytes_sent = send(player.client_socket, response.c_str(), response.size(), 0);
    return bytes_sent > 0;
}

// Main function to handle a client connection
void handle_client(int client_socket, sockaddr_in client_addr) {
    char buffer[1024] = {0};
    int bytes_read = read(client_socket, buffer, 1024);
    
    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }
    
    std::string request(buffer);
    size_t colon1 = request.find(':');
    size_t colon2 = request.find(':', colon1+1);
    size_t colon3 = request.find(':', colon2+1);
    
    if (colon1 == std::string::npos || colon2 == std::string::npos || colon3 == std::string::npos) {
        const char* response = "INVALID_REQUEST";
        send(client_socket, response, strlen(response), 0);
        close(client_socket);
        return;
    }
    
    Player player;
    player.username = request.substr(0, colon1);
    player.mmr = std::stoi(request.substr(colon1+1, colon2-colon1-1));
    player.udp_port = std::stoi(request.substr(colon2+1, colon3-colon2-1));
    player.tcp_port = std::stoi(request.substr(colon3+1));
    player.address = client_addr;
    player.client_socket = client_socket;
    
    // Set client socket as non-blocking
    set_nonblocking(client_socket);
    
    Player matched_player;
    bool found_match = false;
    
    {
        std::lock_guard<std::mutex> lock(players_mutex);
        
        // Check if there are any waiting players
        if (!waiting_players.empty()) {
            matched_player = waiting_players[0];
            waiting_players.erase(waiting_players.begin());
            found_match = true;
            
            // Create a pending match
            PendingMatch match = {matched_player, player, false};
            pending_matches[matched_player.username] = match;
            pending_matches[player.username] = match;
        } else {
            // No match found, add player to waiting list
            waiting_players.push_back(player);
            const char* response = "WAITING";
            send(client_socket, response, strlen(response), 0);
            
            // Don't close the socket - keep it open for match notification
            return;
        }
    }
    
    if (found_match) {
        // Send match notifications to both players
        bool p1_notified = send_match_notification(player, matched_player);
        bool p2_notified = send_match_notification(matched_player, player);
        
        if (p1_notified && p2_notified) {
            std::cout << "Match made between " << player.username 
                      << " and " << matched_player.username << std::endl;
            
            // Mark match as notified
            std::lock_guard<std::mutex> lock(players_mutex);
            pending_matches[player.username].notification_sent = true;
            pending_matches[matched_player.username].notification_sent = true;
        } else {
            std::cerr << "Failed to notify players about match" << std::endl;
        }
    }
    
    // Don't close client_socket yet - keep it open for potential future notifications
}

// Thread to periodically check for pending matches and send notifications
void match_notification_thread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        std::lock_guard<std::mutex> lock(players_mutex);
        for (auto it = pending_matches.begin(); it != pending_matches.end();) {
            PendingMatch& match = it->second;
            
            if (!match.notification_sent) {
                // Try to send notifications
                bool p1_notified = send_match_notification(match.player1, match.player2);
                bool p2_notified = send_match_notification(match.player2, match.player1);
                
                if (p1_notified && p2_notified) {
                    std::cout << "Match notification resent successfully" << std::endl;
                    match.notification_sent = true;
                } else {
                    std::cerr << "Failed to send match notification" << std::endl;
                }
            }
            
            // Clean up completed matches after a while
            if (match.notification_sent) {
                // In a real implementation, we'd wait longer and track game completion
                it = pending_matches.erase(it);
            } else {
                ++it;
            }
        }
    }
}

// Thread to clean up disconnected waiting players
void cleanup_thread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        std::lock_guard<std::mutex> lock(players_mutex);
        for (auto it = waiting_players.begin(); it != waiting_players.end();) {
            // Send a ping to check if client is still connected
            char ping = 0;
            if (send(it->client_socket, &ping, 0, MSG_NOSIGNAL) < 0) {
                std::cout << "Player " << it->username << " disconnected while waiting" << std::endl;
                close(it->client_socket);
                it = waiting_players.erase(it);
            } else {
                ++it;
            }
        }
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options to allow address reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Pong server running on port " << PORT << std::endl;
    
    // Start maintenance threads
    std::thread notify_thread(match_notification_thread);
    notify_thread.detach();
    
    std::thread cleanup(cleanup_thread);
    cleanup.detach();
    
    while (true) {
        sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        
        if (client_socket < 0) {
            perror("accept");
            continue;
        }
        
        std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) 
                  << ":" << ntohs(client_addr.sin_port) << std::endl;
        
        std::thread(handle_client, client_socket, client_addr).detach();
    }
    
    return 0;
}