#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <map>
#include <iostream>

class HttpRequest {
private:
    std::string method;      // GET, POST, PUT, DELETE
    std::string path;        // /index.html or /api/resource
    std::string version;     // HTTP/1.1
    std::map<std::string, std::string> headers;  // Host, User-Agent, etc.
    std::string body;        // Request body (for POST/PUT)

public:
    // Constructors
    HttpRequest();
    HttpRequest(const std::string& method, const std::string& path);
    
    // Getters
    std::string getMethod() const { return method; }
    std::string getPath() const { return path; }
    std::string getVersion() const { return version; }
    std::string getBody() const { return body; }
    std::string getHeader(const std::string& name) const;
    const std::map<std::string, std::string>& getHeaders() const { return headers; }
    
    // Setters
    void setMethod(const std::string& method) { this->method = method; }
    void setPath(const std::string& path) { this->path = path; }
    void setVersion(const std::string& version) { this->version = version; }
    void setBody(const std::string& body) { this->body = body; }
    void setHeader(const std::string& name, const std::string& value);
    
    // Utility methods
    bool isValid() const;
    bool hasHeader(const std::string& name) const;
    std::string toString() const;  // For debugging

    // Keep-alive support
    bool wantsKeepAlive() const;
    bool isHttp11() const;
    
    // HTTP method helpers
    bool isGET() const { return method == "GET"; }
    bool isPOST() const { return method == "POST"; }
    bool isPUT() const { return method == "PUT"; }
    bool isDELETE() const { return method == "DELETE"; }
};

#endif