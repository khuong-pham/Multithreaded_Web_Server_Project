#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>
#include <vector>
#include "HttpRequest.h"

// Example HTTP request
/*
    POST /api/users HTTP/1.1
    Host: www.example.com
    User-Agent: CustomClient/1.2
    Content-Type: application/json
    Content-Length: 47
    Connection: keep-alive

    {
    "username": "William",
    "age": 25
    }

    Raw String:

    std::string request =
    "POST /api/users HTTP/1.1\r\n"
    "Host: www.example.com\r\n"
    "User-Agent: CustomClient/1.2\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 47\r\n"
    "Connection: keep-alive\r\n"
    "\r\n"
    "{\n"
    "  \"username\": \"William\",\n"
    "  \"age\": 25\n"
    "}";
*/

class HttpParser{
    public:
        // Parsing method
        static HttpRequest parse(const std::string& raw_request);

        //Validation method
        static bool isValidHttpRequest(const std::string& raw_request);

    private:
        // Helper methods
        static std::vector<std::string> splitLines(const std::string& text);
        static std::pair<std::string, std::string> parseRequestLine(const std::string& line);
        static std::string trim(const std::string& str);
        static std::string toLowerCase(const std::string& str);
    };
#endif // HTTP_PARSER_H