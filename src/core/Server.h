#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdexcept>
#include "HttpRequest.h"
#include "HttpParser.h"
#include "FileHandler.h"
#include "ResponseGenerator.h"
#include "ThreadPool.h"
#include "Connection.h"
#include <FileCache.h>

class Server {
private:
    int server_socket;
    int port;
    struct sockaddr_in server_addr;
    bool running;
    FileHandler file_handler;  // Add file handler
    std::unique_ptr<ThreadPool> thread_pool;  // Thread pool for handling requests

    // High traffic control
    std::atomic<int> active_connections;
    int max_keepalive_connections;

    // Helper methods
    void setupSocket();
    void bindSocket();
    void startListening();
    void handleClient(int client_socket);
    void handleConnection(std::unique_ptr<Connection> connection);       
    std::string routeRequest(const HttpRequest& request);
    std::string addKeepAliveHeaders(const std::string& response, bool keep_alive, const Connection* connection);
    std::string reasonToString(ConnectionEndReason reason);
    int getActiveConnections() const { return active_connections.load(); }
    int getMaxKeepAliveConnections() const { return max_keepalive_connections; }

    void printPeriodicStats();
public:
    /*Constructor & Destructor*/
    Server(int port = 8080);
    ~Server();
    
    void start();
    void stop();
    int getPort() const { return port; }
    bool isRunning() const { return running; }
};

#endif // SERVER_H