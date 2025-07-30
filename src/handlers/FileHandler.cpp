#include "FileHandler.h"
#include "FileCache.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <algorithm>

FileHandler::FileHandler(const std::string& root) : document_root(root) {
    initializeMimeTypes();
    std::cout << "[FileHandler] Initialized with document root: " << document_root << std::endl;
}

void FileHandler::initializeMimeTypes() {
    // Common MIME types
    mime_types[".html"] = "text/html";
    mime_types[".htm"] = "text/html";
    mime_types[".css"] = "text/css";
    mime_types[".js"] = "application/javascript";
    mime_types[".json"] = "application/json";
    mime_types[".xml"] = "application/xml";
    mime_types[".txt"] = "text/plain";
    
    // Images
    mime_types[".png"] = "image/png";
    mime_types[".jpg"] = "image/jpeg";
    mime_types[".jpeg"] = "image/jpeg";
    mime_types[".gif"] = "image/gif";
    mime_types[".bmp"] = "image/bmp";
    mime_types[".ico"] = "image/x-icon";
    mime_types[".svg"] = "image/svg+xml";
    
    // Fonts
    mime_types[".woff"] = "font/woff";
    mime_types[".woff2"] = "font/woff2";
    mime_types[".ttf"] = "font/ttf";
    mime_types[".otf"] = "font/otf";
    
    // Documents
    mime_types[".pdf"] = "application/pdf";
    mime_types[".zip"] = "application/zip";
    
    std::cout << "[FileHandler] Loaded " << mime_types.size() << " MIME types" << std::endl;
}

std::string FileHandler::getMimeType(const std::string& file_path) {
    std::string extension = getFileExtension(file_path);
    
    auto it = mime_types.find(extension);
    if (it != mime_types.end()) {
        return it->second;
    }
    
    // Default MIME type for unknown files
    return "application/octet-stream";
}

std::string FileHandler::getFileExtension(const std::string& file_path) {
    size_t dot_pos = file_path.find_last_of('.');
    if (dot_pos != std::string::npos) {
        std::string ext = file_path.substr(dot_pos);
        // Convert to lowercase
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
    return "";
}

bool FileHandler::fileExists(const std::string& file_path) {
    std::ifstream file(file_path);
    return file.good();
}

bool FileHandler::isValidPath(const std::string& path) {
    // Security check: prevent directory traversal attacks
    if (path.find("..") != std::string::npos) {
        std::cout << "[FileHandler] Security: Blocked directory traversal attempt: " << path << std::endl;
        return false;
    }
    
    // Check for null bytes (security)
    if (path.find('\0') != std::string::npos) {
        std::cout << "[FileHandler] Security: Blocked null byte in path: " << path << std::endl;
        return false;
    }
    
    return true;
}

std::string FileHandler::readFileContent(const std::string& file_path, bool& success) {
    std::ifstream file(file_path, std::ios::binary);
    success = false;
    
    if (!file.is_open()) {
        std::cout << "[FileHandler] Failed to open file: " << file_path << std::endl;
        return "";
    }
    
    // Read file content
    std::ostringstream content_stream;
    content_stream << file.rdbuf();
    file.close();
    
    success = true;
    std::string content = content_stream.str();
    std::cout << "[FileHandler] Read file: " << file_path << " (" << content.length() << " bytes)" << std::endl;
    
    return content;
}

std::size_t FileHandler::getFileSize(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        return file.tellg();
    }
    return 0;
}

bool FileHandler::canServeFile(const std::string& request_path) {
    if (!isValidPath(request_path)) {
        return false;
    }
    
    std::string normalized_path = request_path;
    if (normalized_path == "/") {
        normalized_path = "/index.html";
    }
    
    // Check cache with normalized path
    auto& cache = FileCacheManager::get_instance();
    auto cached_file = cache.get(normalized_path);
    if (cached_file) {
        return true;  // File is in cache
    }
    
    // Then check disk
    std::string full_path = document_root + normalized_path;
    return fileExists(full_path);
}

std::string FileHandler::serveFile(const std::string& request_path) {
    std::cout << "[FileHandler] Serving file request: " << request_path << std::endl;
    
    // Security validation
    if (!isValidPath(request_path)) {
        return createErrorResponse(403, "Forbidden", "Invalid file path");
    }
    
    // Handle root path
    std::string file_path = request_path;
    if (file_path == "/") {
        std::cout << "[FileHandler] Root path requested, serving index.html" << std::endl;
        file_path = "/index.html";
    }
    
    auto& cache = FileCacheManager::get_instance();
    auto cached_file = cache.get(file_path);

    if (cached_file) {
        // Cache hit - serve from memory!
        std::cout << "[FileHandler] Serving from cache: " << file_path << std::endl;
        return buildHttpResponse(*cached_file);
    }

    // Cache miss - load from disk and cache it
    std::cout << "[FileHandler] Cache miss, loading from disk: " << file_path << std::endl;


    // Build full file path
    std::string full_path = document_root + file_path;
    std::cout << "[FileHandler] Full path: " << full_path << std::endl;
    
    // Check if file exists
    if (!fileExists(full_path)) {
        return createErrorResponse(404, "Not Found", "File not found: " + request_path);
    }
    
    // Cache miss - load from disk and cache it
    std::cout << "[FileHandler] Cache miss, loading from disk: " << file_path << std::endl;
    
    // Build full file path
    std::cout << "[FileHandler] Full path: " << full_path << std::endl;
    
    // Read file content
    bool read_success = false;
    std::string file_content = readFileContent(full_path, read_success);

    if (!read_success) {
        return createErrorResponse(500, "Internal Server Error", "Failed to read file");
    }
    
    // Get MIME type
    std::string content_type = getMimeType(full_path);
    
    // Create cached file object
    CachedFile new_cached_file(file_content, content_type);
    
    // Try to add to cache
    bool cached = cache.put(file_path, new_cached_file);
    if (cached) {
        std::cout << "[FileHandler] File cached successfully: " << file_path << std::endl;
    } else {
        std::cout << "[FileHandler] File too large to cache: " << file_path << std::endl;
    }
    
    // Serve the file
    std::cout << "[FileHandler] Served file successfully: " << request_path 
              << " (Content-Type: " << content_type << ")" << std::endl;
    
    return buildHttpResponse(new_cached_file);
}

std::string FileHandler::buildHttpResponse(const CachedFile& cached_file) {
    std::string response;
    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + cached_file.mime_type + "\r\n";
    response += "Content-Length: " + std::to_string(cached_file.content.length()) + "\r\n";
    response += "Server: CustomHTTPServer/1.0\r\n";
    response += "Cache-Control: max-age=3600\r\n";  // Cache for 1 hour
    response += "Connection: close\r\n";
    response += "\r\n";
    response += cached_file.content;
    
    return response;
}

std::string FileHandler::createErrorResponse(int status_code, const std::string& status_text, const std::string& message) {
    std::string html_body = R"(
<!DOCTYPE html>
<html>
<head>
    <title>)" + std::to_string(status_code) + " " + status_text + R"(</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; text-align: center; }
        h1 { color: #e74c3c; }
    </style>
</head>
<body>
    <h1>)" + std::to_string(status_code) + " - " + status_text + R"(</h1>
    <p>)" + message + R"(</p>
    <a href="/">Back to Home</a>
</body>
</html>)";
    
    std::string response;
    response += "HTTP/1.1 " + std::to_string(status_code) + " " + status_text + "\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + std::to_string(html_body.length()) + "\r\n";
    response += "Server: CustomHTTPServer/1.0\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += html_body;
    
    return response;
}

// Add method to get cache statistics
void FileHandler::printCacheStats() const {
    auto& cache = FileCacheManager::get_instance();
    auto stats = cache.getStats();
    
    std::cout << "[FileHandler] Cache Statistics:" << std::endl;
    std::cout << "  Entries: " << stats.entries << std::endl;
    std::cout << "  Size: " << (stats.size_bytes / 1024) << "KB / " 
              << (stats.capacity_bytes / 1024) << "KB" << std::endl;
    std::cout << "  Hit Ratio: " << (stats.hit_ratio * 100) << "%" << std::endl;
    std::cout << "  Hits: " << stats.hits << ", Misses: " << stats.misses << std::endl;
}