#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include "core/Server.h"

class ConnectionTest : public ::testing::Test {
    protected:
        int test_socket;
        std::string test_ip;
        
        void SetUp() override{
            test_socket = socket(AF_INET, SOCK_STREAM, 0);
            test_ip = "127.0.0.1";
        }

        void TearDown() override {
            if (test_socket != -1) {
                close(test_socket);
            }
        }
};

// Test basic connection creation
TEST_F(ConnectionTest, BasicCreation)
{
    Connection conn(test_socket, test_ip);
    
    EXPECT_EQ(conn.getSocketFd(), test_socket);
    EXPECT_EQ(conn.getClientIp(), test_ip);
    EXPECT_EQ(conn.getCurrentRequests(), 0);
    EXPECT_TRUE(conn.canContinue());
}

// Test request counting and limits
TEST_F(ConnectionTest, RequestCounting)
{
    Connection conn(test_socket, test_ip);
    
    conn.setMaxRequests(3);
    
    EXPECT_EQ(conn.getCurrentRequests(), 0);
    EXPECT_FALSE(conn.hasReachedMaxRequests());
    
    for (int i = 0; i < 2; ++i) {
        conn.incrementRequestCount();
        EXPECT_EQ(conn.getCurrentRequests(), i + 1);
        EXPECT_FALSE(conn.hasReachedMaxRequests());
    }
    
    conn.incrementRequestCount();
    EXPECT_EQ(conn.getCurrentRequests(), 3);
    EXPECT_TRUE(conn.hasReachedMaxRequests());
}

// Test HTTP Request validation
TEST_F(ConnectionTest, HttpRequestValidation)
{
    HttpRequest request;
    request.setMethod("GET");
    request.setPath("/test");
    EXPECT_TRUE(request.isValid());

    HttpRequest invalid;
    invalid.setPath("/test");  // No method
    EXPECT_FALSE(invalid.isValid());
}