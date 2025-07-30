#ifndef RESPONSE_GENERATOR_H
#define RESPONSE_GENERATOR_H

#include <string>

class ResponseGenerator {
public:
    // Static HTML page generators
    static std::string createHomePageResponse(int port);
    static std::string createAboutPageResponse();
    static std::string createStatusPageResponse(int port);
    
    // Error response generators
    static std::string createErrorResponse(int status_code, const std::string& message);
    static std::string create404Response();
    static std::string create400Response(); 
    static std::string create500Response();
    
    // Basic HTTP response wrapper
    static std::string createHttpResponse(const std::string& body, 
                                        const std::string& content_type = "text/html",
                                        int status_code = 200,
                                        const std::string& status_text = "OK");

private:
    // Helper methods
    static std::string getStatusText(int status_code);
    static std::string createErrorHtml(int status_code, const std::string& title, const std::string& message);
};

#endif