#include "Server.h"
#include <poll.h>
#include <fcntl.h>
#include <algorithm>

Server::Server(int port) : port(port), running(false), server_socket(-1), file_handler("./public"), active_connections(0), max_keepalive_connections(100) {
    std::cout << "[Server] Initializing server on port " << port << std::endl;

    // Initialize thread pool with hardware concurrency size;
    size_t thread_count = std::thread::hardware_concurrency();
    if(thread_count == 0 ) thread_count = 4;

    max_keepalive_connections = thread_count / 2;  // Only allow 50% of threads for keep-alive
    if (max_keepalive_connections < 1) max_keepalive_connections = 1;

    thread_pool = std::make_unique<ThreadPool>(thread_count);
    std::cout << "[Server] Thread pool initialized with " << thread_count << " threads" << std::endl;
}

Server::~Server() {
    stop();
}

void Server::setupSocket() {
    /* Create Socket with type SOCK_STREAM*/
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket() failed");
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    }
    
    std::cout << "[Server] Socket created successfully (fd=" << server_socket << ")" << std::endl;
    
    /* Set REUSEADDR option to avoid "Address aldready in use"*/
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt() failed");
        close(server_socket);
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
    }
    
    std::cout << "[Server] Socket options set successfully" << std::endl;
}

void Server::bindSocket() {
    /* Init and set parameters */
    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP
    server_addr.sin_port = htons(port);        // Convert to network byte order
    
    std::cout << "[Server] Attempting to bind to port " << port << std::endl;
    
    /* Bind socket to address */ 
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() failed");
        close(server_socket);
        throw std::runtime_error("Failed to bind socket to port " + std::to_string(port) + 
                                ": " + std::string(strerror(errno)));
    }
    
    std::cout << "[Server] Socket bound to port " << port << " successfully" << std::endl;
}

void Server::startListening() {
    /* Start listening for connections (backlog = 10) */
    if (listen(server_socket, SOMAXCONN) < 0) {
        close(server_socket);
        throw std::runtime_error("Failed to listen on socket");
    }
    
    std::cout << "[Server] Listening for connections..." << std::endl;
}

void Server::start() {
    try {
        setupSocket();
        bindSocket();
        startListening();
        
        running = true;
        std::cout << "[Server] Server started successfully on http://localhost:" << port << std::endl;
        
        /* Main loop at Server side */
        while (running) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            printPeriodicStats();
            
            // Accept incoming connection
            int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
            
            if (client_socket < 0) {
                if (running) {  // Only log error if server is still supposed to be running
                    std::cerr << "[Server] Failed to accept client connection" << std::endl;
                }
                continue;
            }
            
            /* Get client IP */
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            std::cout << "[Server] New connection from " << client_ip << std::endl;
            
            // Handle client request using thread pool (NON BLOCKING)
            thread_pool->enqueue([this, client_socket, client_ip]() {
                std::cout << "[ThreadPool] Processing connection from " << client_ip <<
                    " on thread " << std::this_thread::get_id() << std::endl;
                
                auto connection = std::make_unique<Connection>(client_socket, client_ip);
                try {
                    handleConnection(std::move(connection));
                } catch (const std::exception& e) {
                    std::cerr << "[ThreadPool] Error handling client " << client_ip 
                              << ": " << e.what() << std::endl;
                }
            }); 
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[Server] Error: " << e.what() << std::endl;
        stop();
    }
}

void Server::handleClient(int client_socket) {
    char buffer[4096] = {0};
    
    // Read request from client
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    
    if (bytes_read > 0) {
        std::string raw_request(buffer);
        std::cout << "[Server] Received request (" << bytes_read << " bytes)" << std::endl;
        
        // Parse HTTP request
        HttpRequest request = HttpParser::parse(raw_request);
        
        if (request.isValid()) {
            std::cout << "[Server] Valid HTTP request parsed:" << std::endl;
            std::cout << request.toString() << std::endl;
            
            try {
                // Route request and generate response
                std::string response = routeRequest(request);
                
                // Send response to client
                send(client_socket, response.c_str(), response.length(), 0);
                std::cout << "[Server] Response sent successfully" << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "[Server] Error processing request: " << e.what() << std::endl;
                
                // Send 500 Internal Server Error
                std::string error_response = ResponseGenerator::create500Response();
                send(client_socket, error_response.c_str(), error_response.length(), 0);
            }
            
        } else {
            std::cout << "[Server] Invalid HTTP request received" << std::endl;
            
            // Send 400 Bad Request
            std::string error_response = ResponseGenerator::create400Response();
            send(client_socket, error_response.c_str(), error_response.length(), 0);
        }
    }
    else {
        std::cout << "[Server] No data received from client" << std::endl;
    }

    // Close client connection
    close(client_socket);
    std::cout << "[Server] Client connection closed" << std::endl;
}

void Server::handleConnection(std::unique_ptr<Connection> connection) {
    // Increment active connections
    active_connections.fetch_add(1);
    int current_active = active_connections.load();
    
    std::cout << "[Server] Starting connection handling: " 
              << connection->getStatusString() 
              << " | Active: " << current_active << std::endl;;
    
    connection->setMaxRequests(5); 
    connection->setTimeout(std::chrono::seconds(3));

    auto terminate = [&](ConnectionEndReason reason) {
        std::cout << "[Server] Terminating connection: " << reasonToString(reason) << " | " << connection->getStatusString() << std::endl;
        connection->markForClosing();
    };

    while (connection->canContinue()) {
        char buffer[4096] = {0};
        connection->setState(ConnectionState::READING);
        connection->updateActivity();

        // Set socket timeout for reading (blocking read)
        struct timeval tv;
        tv.tv_sec = static_cast<int>(connection->getTimeout().count());
        tv.tv_usec = 0;
        if (setsockopt(connection->getSocketFd(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
            terminate(ConnectionEndReason::ReadError);
            break;
        }

        // Read request from client (blocking, with timeout)
        ssize_t bytes_read = read(connection->getSocketFd(), buffer, sizeof(buffer) - 1);
        if (bytes_read == 0) {
            terminate(ConnectionEndReason::ClientClosed);
            break;
        } else if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == ETIMEDOUT) {
                terminate(ConnectionEndReason::Timeout);
            } else {
                terminate(ConnectionEndReason::ReadError);
            }
            break;
        }
        connection->setState(ConnectionState::PROCESSING);
        connection->incrementRequestCount();
        connection->updateActivity();
        std::string raw_request(buffer, bytes_read);
        std::cout << "[Server] Processing request " << connection->getCurrentRequests() 
                  << "/" << connection->getMaxRequests()     
                  << " from " << connection->getClientIp()
                  << " (" << bytes_read << " bytes)" << std::endl;

        // Parse HTTP request
        HttpRequest request = HttpParser::parse(raw_request);
        if (!request.isValid()) {
            std::cout << "[Server] Invalid HTTP request" << std::endl;
            std::string error_response = ResponseGenerator::create400Response();
            send(connection->getSocketFd(), error_response.c_str(), error_response.length(), 0);
            terminate(ConnectionEndReason::BadRequest);
            break;
        }
        try {
            connection->setState(ConnectionState::WRITING);
            std::string response = routeRequest(request);
            bool client_wants_keepalive = request.wantsKeepAlive();
            bool server_can_continue = connection->canContinue();
            int current_load = active_connections.load();
            bool traffic_allows_keepalive = (current_load <= max_keepalive_connections);
        
            // Final decision
            bool use_keepalive = client_wants_keepalive && 
                               server_can_continue && 
                               traffic_allows_keepalive;


           std::cout << "[Server] Keep-alive analysis:\n"
                      << "  - Client wants: " << (client_wants_keepalive ? "yes" : "no") << std::endl
                      << "  - Server can continue: " << (server_can_continue ? "yes" : "no") << std::endl
                      << "  - Current load: " << current_load << "/" << max_keepalive_connections << std::endl
                      << "  - Traffic control: " << (traffic_allows_keepalive ? "allows" : "blocks") << std::endl
                      << "  - Max requests reached: " << (connection->hasReachedMaxRequests() ? "yes" : "no") << std::endl
                      << "  - Final decision: " << (use_keepalive ? "KEEP-ALIVE" : "CLOSE") << std::endl;
            
            response = addKeepAliveHeaders(response, use_keepalive, connection.get());
            ssize_t sent = send(connection->getSocketFd(), response.c_str(), response.length(), 0);
            if (sent < 0) {
                terminate(ConnectionEndReason::SendError);
                break;
            }
            std::cout << "[Server] Response sent (" << sent << " bytes)" << std::endl;
            if (!use_keepalive) {
                ConnectionEndReason reason;
                if (connection->hasReachedMaxRequests()) {
                    reason = ConnectionEndReason::MaxRequests;
                } else if (!traffic_allows_keepalive) {
                    reason = ConnectionEndReason::KeepAliveNotAllowed;
                } else {
                    reason = ConnectionEndReason::KeepAliveNotAllowed;
                }
                terminate(reason);
                break;
            }
            connection->setState(ConnectionState::KEEP_ALIVE);
            std::cout << "[Server] Connection status: " << connection->getStatusString() << std::endl;
            std::cout << "[Server] Waiting for next request (timeout in " 
                      << connection->getTimeout().count() << "s)..." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Server] Error processing request: " << e.what() << std::endl;
            std::string error_response = ResponseGenerator::create500Response();
            send(connection->getSocketFd(), error_response.c_str(), error_response.length(), 0);
            terminate(ConnectionEndReason::Exception);
            break;
        }
    }
    connection->setState(ConnectionState::CLOSING);

    active_connections.fetch_sub(1);
    int remaining_active = active_connections.load();
    
    std::cout << "[Server] Keep-alive connection ended: " << connection->getStatusString() 
              << " | Remaining active: " << remaining_active << std::endl;

   
}

std::string Server::addKeepAliveHeaders(const std::string& response, bool keep_alive, const Connection* connection) {
    // Find the end of headers (empty line)
    size_t headers_end = response.find("\r\n\r\n");
    if (headers_end == std::string::npos) {
        return response;  // Malformed response
    }
    
    std::string headers = response.substr(0, headers_end);
    std::string body = response.substr(headers_end);
    
    // Add keep-alive headers
    if (keep_alive) {
        headers += "\r\nConnection: keep-alive";
        headers += "\r\nKeep-Alive: timeout=" + std::to_string(connection->getTimeout().count()) + 
                   ", max=" + std::to_string(connection->getMaxRequests());
    } else {
        headers += "\r\nConnection: close";
    }
    
    return headers + body;
}

std::string Server::routeRequest(const HttpRequest& request)
{
    std::string path = request.getPath();
    std::cout <<"[Server] Routing request to path: " << path << std::endl;  

    // Try to serve static files for everything else
    if (file_handler.canServeFile(path)) {
        std::cout << "[Server] Serving static file: " << path << std::endl;
        return file_handler.serveFile(path);
    }

    if (path == "/about") {
        std::cout << "[Server] Serving about page" << std::endl;
        return ResponseGenerator::createAboutPageResponse(); 
    } 
    else if (path == "/status") {
        std::cout << "[Server] Serving status page" << std::endl;
        return ResponseGenerator::createStatusPageResponse(port);
    }

    

    std::cout << "[Server] Path not found: " << path << std::endl;
    return ResponseGenerator::create404Response();
}

// Add this method to print periodic cache statistics
void Server::printPeriodicStats() {
    static auto last_stats_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    
    // Print stats every 30 seconds
    if (std::chrono::duration_cast<std::chrono::seconds>(now - last_stats_time).count() >= 30) {
        std::cout << "\n=== SERVER STATISTICS ===" << std::endl;
        std::cout << "[Server] Active connections: " << active_connections.load() << std::endl;
        
        // Print cache statistics
        file_handler.printCacheStats();
        
        // Print LRU cache detailed state (for debugging)
        auto& cache = FileCacheManager::get_instance();
        cache.print_cache_state();
        
        std::cout << "========================\n" << std::endl;
        
        last_stats_time = now;
    }
}

// Helper function to convert ConnectionEndReason to string
std::string Server::reasonToString(ConnectionEndReason reason) {
    switch (reason) {
        case ConnectionEndReason::ClientClosed: return "CLIENT_CLOSED";
        case ConnectionEndReason::Timeout: return "TIMEOUT";
        case ConnectionEndReason::ReadError: return "READ_ERROR";
        case ConnectionEndReason::SendError: return "SEND_ERROR";
        case ConnectionEndReason::BadRequest: return "BAD_REQUEST";
        case ConnectionEndReason::Exception: return "EXCEPTION";
        case ConnectionEndReason::MaxRequests: return "MAX_REQUESTS";
        case ConnectionEndReason::KeepAliveNotAllowed: return "KEEPALIVE_NOT_ALLOWED";
        default: return "UNKNOWN";
    }
}

void Server::stop() {
    if (running) {
        running = false;
        std::cout << "[Server] Stopping server..." << std::endl;
        
        // Show thread pool statistics
        if (thread_pool) {
            thread_pool->printStatus();
        }
    }
    
    if (server_socket != -1) {
        close(server_socket);
        server_socket = -1;
        std::cout << "[Server] Socket closed" << std::endl;
    }
    
    // Shutdown thread pool
    if (thread_pool) {  
        std::cout << "[Server] Shutting down thread pool..." << std::endl;
        thread_pool->shutdown();
        std::cout << "[Server] Thread pool shut down complete" << std::endl;
    }
}