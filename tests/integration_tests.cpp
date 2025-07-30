#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class IntegrationTest : public ::testing::Test {
    protected:
        int connectToServer()
        {
            int sock  = socket(AF_INET, SOCK_STREAM, 0);
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

        std::string sendRequest(const std::string& request) {
            int sock = connectToServer();
            if (sock < 0) return "";
            
            send(sock, request.c_str(), request.length(), 0);
            
            char buffer[4096];
            int received = recv(sock, buffer, sizeof(buffer)-1, 0);
            close(sock);
            
            if (received > 0) {
                buffer[received] = '\0';
                return std::string(buffer);
            }
        return "";
    }
};

//Test Baisc Get request
TEST_F(IntegrationTest, BasicGetRequest) {
    std::string request = "GET /about HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string response = sendRequest(request);

    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response.substr(0, 15), "HTTP/1.1 200 OK");
}

// Test 404 Not Found
TEST_F(IntegrationTest, NotFoundRequest) {
    std::string request = "GET /nonexistent HTTP/1.1\r\nHost: localhost\r\n\r\n";
    std::string response = sendRequest(request);
    
    ASSERT_FALSE(response.empty());
    EXPECT_EQ(response.substr(0, 12), "HTTP/1.1 404");
}

// Test Keep-Alive Connection
TEST_F(IntegrationTest, KeepAliveConnection) {
    int sock = connectToServer();
    ASSERT_GT(sock, 0);
    
    // First request with keep-alive
    std::string request1 = "GET /about HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n";
    send(sock, request1.c_str(), request1.length(), 0);
    
    char buffer1[8192];
    int received1 = recv(sock, buffer1, sizeof(buffer1)-1, 0);
    ASSERT_GT(received1, 0);
    
    std::string response1(buffer1, received1);
    EXPECT_EQ(response1.substr(0, 12), "HTTP/1.1 200");
    
    // Verify server responds with keep-alive header
    EXPECT_NE(response1.find("Connection: keep-alive"), std::string::npos);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Second request on same connection
    std::string request2 = "GET /about HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n";
    int sent = send(sock, request2.c_str(), request2.length(), 0);
    ASSERT_GT(sent, 0);
    
    char buffer2[8192];
    int received2 = recv(sock, buffer2, sizeof(buffer2)-1, 0);
    
    close(sock);
    
    ASSERT_GT(received2, 0);
    
    std::string response2(buffer2, received2);
    EXPECT_EQ(response2.substr(0, 12), "HTTP/1.1 200");
    EXPECT_NE(response2.find("Connection: keep-alive"), std::string::npos);
}