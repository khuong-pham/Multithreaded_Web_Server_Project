#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class LoadTest : public ::testing::Test {
protected:
    int connectToServer() {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return -1;
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8080);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock);
            return -1;
        }
        return sock;
    }
    
    bool sendRequest() {
        int sock = connectToServer();
        if (sock < 0) return false;
        
        std::string request = "GET /about HTTP/1.1\r\nHost: localhost\r\n\r\n";
        send(sock, request.c_str(), request.length(), 0);
        
        char buffer[1024];
        int received = recv(sock, buffer, sizeof(buffer)-1, 0);
        close(sock);
        
        return received > 0 && std::string(buffer).find("HTTP/1.1 200") == 0;
    }
};

// Test concurrent load
TEST_F(LoadTest, LightLoad) {
    const int clients = 5;
    const int requests_per_client = 3;
    
    std::atomic<int> success{0};
    std::atomic<int> failed{0};
    std::vector<std::thread> threads;
    
    for (int i = 0; i < clients; ++i) {
        threads.emplace_back([&, requests_per_client]() {
            for (int j = 0; j < requests_per_client; ++j) {
                if (sendRequest()) {
                    success++;
                } else {
                    failed++;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    int total = clients * requests_per_client;
    EXPECT_GT(success.load(), 0);
    EXPECT_LT(failed.load(), total * 0.1); // Less than 10% failure rate
    
    std::cout << "Light Load: " << success.load() << "/" << total << " success" << std::endl;
}