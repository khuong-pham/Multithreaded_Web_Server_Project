#include <iostream>
#include <unistd.h>
#include <sstream>
#include "Connection.h"

Connection::Connection(int socket, const std::string& ip) : socket_fd(socket), 
    state(ConnectionState::READING), client_ip(ip), max_requests(10),  
    current_requests(0), timeout(std::chrono::seconds(30)), should_close(false) {

        updateActivity();
        std::cout << "[Connection] New connection created: " << client_ip 
                    << " on socket " << socket_fd << std::endl;               
}

Connection::~Connection() {
    if (socket_fd != -1) {
        close(socket_fd);
        std::cout << "[Connection] Connection closed for " << client_ip 
                  << " on socket " << socket_fd << ", request=" << current_requests << std::endl;
    }
}

bool Connection::hasReachedMaxRequests() const {
    return current_requests >= max_requests;
}

void Connection::setState(ConnectionState new_state) {
    std::cout << "[Connection] " << client_ip << " state: " 
              << static_cast<int>(state) << " -> " << static_cast<int>(new_state) << std::endl;
    state = new_state;
    updateActivity();
}

void Connection::markForClosing() {
    should_close.store(true);
    std::cout << "[Connection] " << client_ip << " marked for closing" << std::endl;
}

void Connection::updateActivity() {
    last_activity = std::chrono::steady_clock::now();
}

void Connection::incrementRequestCount()
{
    current_requests++;
    std::cout << "[Connection] " << client_ip << " request count: " 
              << current_requests << "/" << max_requests << std::endl;
    updateActivity();    
}

std::string Connection::getStatusString() const {
    std::ostringstream oss;
    oss << "Connection{socket=" << socket_fd 
        << ", client=" << client_ip
        << ", state=" << static_cast<int>(state)
        << ", requests=" << current_requests << "/" << max_requests
        << ", keepalive=" << (canContinue() ? "yes" : "no")
        << "}";
    return oss.str();
}

bool Connection::canContinue() const {
    return !should_close.load() && current_requests < max_requests;
}
