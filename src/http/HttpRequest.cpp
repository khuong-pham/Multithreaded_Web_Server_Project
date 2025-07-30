#include "HttpRequest.h"
#include <algorithm>
#include <sstream>

HttpRequest::HttpRequest() : method(""), path(""), version("HTTP/1.1") {
}

HttpRequest::HttpRequest(const std::string& method, const std::string& path) 
    : method(method), path(path), version("HTTP/1.1") {
}

std::string HttpRequest::getHeader(const std::string& name) const {
    auto it = headers.find(name);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

void HttpRequest::setHeader(const std::string& name, const std::string& value) {
    headers[name] = value;
}

bool HttpRequest::hasHeader(const std::string& name) const {
    return headers.find(name) != headers.end();
}

bool HttpRequest::isValid() const {
    // Basic validation
    if (method.empty() || path.empty()) {
        return false;
    }
    
    // Valid HTTP methods
    if (method != "GET" && method != "POST" && method != "PUT" && 
        method != "DELETE" && method != "HEAD" && method != "OPTIONS") {
        return false;
    }
    
    // Path should start with /
    if (path[0] != '/') {
        return false;
    }
    
    return true;
}

std::string HttpRequest::toString() const {
    std::ostringstream oss;
    
    oss << "=== HTTP Request ===" << std::endl;
    oss << "Method: " << method << std::endl;
    oss << "Path: " << path << std::endl;
    oss << "Version: " << version << std::endl;
    
    oss << "Headers:" << std::endl;
    for (const auto& header : headers) {
        oss << "  " << header.first << ": " << header.second << std::endl;
    }
    
    if (!body.empty()) {
        oss << "Body Length: " << body.length() << " bytes" << std::endl;
        if (body.length() < 200) {  // Only show small bodies
            oss << "Body: " << body << std::endl;
        }
    }
    
    oss << "===================" << std::endl;
    
    return oss.str();
}

bool HttpRequest::wantsKeepAlive() const{
    // Check Connection header for keep-alive
    std::string connection = getHeader("Connection");
    if(!connection.empty())
    {
        std::transform(connection.begin(), connection.end(), connection.begin(), ::tolower);
        return connection.find("keep-alive") != std::string::npos;
    }

    // Default to HTTP/1.1 keep-alive
    if(isHttp11()) {
        return true;
    }

    return false;
}

bool HttpRequest::isHttp11() const {
    return version == "HTTP/1.1";
}
