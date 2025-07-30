#include "HttpParser.h"
#include <sstream>
#include <algorithm>
#include <iostream>

HttpRequest HttpParser::parse(const std::string& raw_request) {
    HttpRequest request;
    
    if (raw_request.empty()) {
        std::cerr << "[Parser] Empty request" << std::endl;
        return request;
    }
    
    // Split request into lines
    std::vector<std::string> lines = splitLines(raw_request);
    
    if (lines.empty()) {
        std::cerr << "[Parser] No lines found in request" << std::endl;
        return request;
    }
    
    // Parse request line (first line)
    // Example: "GET /index.html HTTP/1.1"
    std::string request_line = lines[0];
    std::cout << "[Parser] Request line: " << request_line << std::endl;
    
    std::istringstream iss(request_line);
    std::string method, path, version;
    
    if (!(iss >> method >> path >> version)) {
        std::cerr << "[Parser] Failed to parse request line" << std::endl;
        return request;
    }
    
    request.setMethod(method);
    request.setPath(path);
    request.setVersion(version);
    
    std::cout << "[Parser] Parsed - Method: " << method 
              << ", Path: " << path 
              << ", Version: " << version << std::endl;
    
    // Parse headers (lines after request line until empty line)
    size_t i = 1;
    while (i < lines.size() && !lines[i].empty()) {
        std::string header_line = lines[i];
        
        // Find colon separator
        size_t colon_pos = header_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string header_name = trim(header_line.substr(0, colon_pos));
            std::string header_value = trim(header_line.substr(colon_pos + 1));
            
            request.setHeader(header_name, header_value);
            std::cout << "[Parser] Header: " << header_name << " = " << header_value << std::endl;
        }
        
        i++;
    }
    
    // Parse body (after empty line, if exists)
    if (i < lines.size()) {
        std::string body;
        for (size_t j = i + 1; j < lines.size(); j++) {
            body += lines[j];
            if (j < lines.size() - 1) {
                body += "\n";
            }
        }
        
        if (!body.empty()) {
            request.setBody(body);
            std::cout << "[Parser] Body length: " << body.length() << " bytes" << std::endl;
        }
    }
    
    std::cout << "[Parser] Request parsed successfully" << std::endl;
    return request;
}

bool HttpParser::isValidHttpRequest(const std::string& raw_request) {
    if (raw_request.empty()) {
        return false;
    }
    
    // Check if it starts with HTTP method
    std::vector<std::string> methods = {"GET", "POST", "PUT", "DELETE", "HEAD", "OPTIONS"};
    
    for (const std::string& method : methods) {
        if (raw_request.substr(0, method.length()) == method) {
            return true;
        }
    }
    
    return false;
}

std::vector<std::string> HttpParser::splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Remove carriage return if present (Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    
    return lines;
}

std::string HttpParser::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string HttpParser::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}