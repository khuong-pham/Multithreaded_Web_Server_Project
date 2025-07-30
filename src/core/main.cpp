#include <iostream>
#include <csignal>
#include "Server.h"

// Global server instance for signal handling
Server* global_server = nullptr;

// Signal handler for graceful shutdown (Ctrl+C)
void signalHandler(int signal) {
    std::cout << "\n[Main] Received signal " << signal << " - shutting down gracefully..." << std::endl;
    if (global_server) {
        global_server->stop();
    }
    exit(0);
}

int main(int argc, char* argv[]) {
    std::cout << "=== Multithreaded HTTP Web Server ===" << std::endl;
    std::cout << "Author: Your Name" << std::endl;
    std::cout << "Version: 1.0.0 - Basic Socket Implementation" << std::endl;
    std::cout << "=========================================" << std::endl;
    
    // Parse command line arguments
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
            if (port < 1024 || port > 65535) {
                std::cerr << "Error: Port must be between 1024 and 65535" << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: Invalid port number '" << argv[1] << "'" << std::endl;
            return 1;
        }
    }
    
    try {
        // Create server instance
        Server server(port);
        global_server = &server;
        
        // Setup signal handlers for graceful shutdown
        signal(SIGINT, signalHandler);   // Ctrl+C
        signal(SIGTERM, signalHandler);  // Termination signal
        
        std::cout << "\n Starting server..." << std::endl;
        std::cout << " Open your browser and go to: http://localhost:" << port << std::endl;
        std::cout << "  Press Ctrl+C to stop the server\n" << std::endl;
        
        // Start the server (blocking call)
        server.start();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n👋 Server shutdown complete. Goodbye!" << std::endl;
    return 0;
}