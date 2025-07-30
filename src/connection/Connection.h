#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <chrono>
#include <atomic>

enum class ConnectionEndReason {
    Timeout,
    ClientClosed,
    ReadError,
    BadRequest,
    SendError,
    MaxRequests,
    KeepAliveNotAllowed,
    Exception
};

static std::string reasonToString(ConnectionEndReason reason) {
    switch (reason) {
        case ConnectionEndReason::Timeout: return "timeout";
        case ConnectionEndReason::ClientClosed: return "client closed connection";
        case ConnectionEndReason::ReadError: return "read error";
        case ConnectionEndReason::BadRequest: return "bad request";
        case ConnectionEndReason::SendError: return "send error";
        case ConnectionEndReason::MaxRequests: return "max requests reached";
        case ConnectionEndReason::KeepAliveNotAllowed: return "keep-alive not allowed";
        case ConnectionEndReason::Exception: return "exception during processing";
        default: return "unknown";
    }
}

enum class ConnectionState{
    READING,
    PROCESSING,
    WRITING,
    KEEP_ALIVE,
    CLOSING
};

class Connection
{
    private:
        int socket_fd;
        ConnectionState state;
        std::chrono::steady_clock::time_point last_activity;
        std::string client_ip;

        // Keep alive settings
        int max_requests;
        int current_requests;
        std::chrono::seconds timeout;

        std::atomic<bool> should_close;
    
    public:
        Connection(int socket, const std::string& ip);
        ~Connection();

        // Delete copy constructor and copy assignment operators
        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;

        // Connection management
        bool canContinue() const;
        bool canKeepAlive() const;
        bool hasReachedMaxRequests() const;

        // State Management
        void setState(ConnectionState new_state);
        ConnectionState getState() const { return state;}
        void updateActivity();
        void incrementRequestCount();

        // Getters
        int getSocketFd() const { return socket_fd;}
        std::string getClientIp() const { return client_ip; }
        int getCurrentRequests() const { return current_requests; }
        int getMaxRequests() const { return max_requests; }
        std::chrono::seconds getTimeout() const { return timeout; }
        // Connection Control
        void markForClosing(); 
        bool shouldClose() const { return should_close.load(); }

        // Keep Alive Settings
        void setMaxRequests(int max) { max_requests = max; }
        void setTimeout(std::chrono::seconds t) { timeout = t; }

        // Status
        std::string getStatusString() const;
};
#endif // CONNECTION_H