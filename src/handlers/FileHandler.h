#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <map>
#include <fstream>
#include "HttpRequest.h"
#include "FileCache.h"
class FileHandler {
private:
    std::string document_root;
    std::map<std::string, std::string> mime_types;
    
    // Helper methods
    void initializeMimeTypes();
    std::string getMimeType(const std::string& file_path);
    std::string getFileExtension(const std::string& file_path);
    bool fileExists(const std::string& file_path);
    bool isValidPath(const std::string& path);
    std::string readFileContent(const std::string& file_path, bool& success);
    std::size_t getFileSize(const std::string& file_path);
    std::string createErrorResponse(int status_code, const std::string& status_text, const std::string& message);
public:
    FileHandler(const std::string& root = "./public");
    
    // Main file serving method
    std::string serveFile(const std::string& request_path);
    
    // Check if file can be served
    bool canServeFile(const std::string& request_path);
    std::string buildHttpResponse(const CachedFile& cached_file);
    // Utility methods
    std::string getDocumentRoot() const { return document_root; }
    void setDocumentRoot(const std::string& root) { document_root = root; }
    void printCacheStats() const;
};

#endif