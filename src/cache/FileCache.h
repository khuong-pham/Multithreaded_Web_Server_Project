#ifndef FILE_CACHE_H
#define FILE_CACHE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <shared_mutex>
#include <chrono>
#include <iostream>
#include <mutex>

struct CachedFile
{
    std::string content;
    std::string mime_type;
    size_t size_bytes;
    std::chrono::system_clock::time_point cached_time;

    CachedFile(const std::string& content, const std::string& mime_type)
        : content(content), mime_type(mime_type),
          size_bytes(content.size()), cached_time(std::chrono::system_clock::now()) {}
};

// Double Linked List Node for LRU Cache
struct CacheNode
{
    std::string key; // filea Path
    CachedFile file;
    std::shared_ptr<CacheNode> prev;
    std::shared_ptr<CacheNode> next;

    CacheNode(const std::string& key, const CachedFile& data)
        : key(key), file(data), prev(nullptr), next(nullptr) {}
};

class LRUFileCache
{
    private:
        std::unordered_map<std::string, std::shared_ptr<CacheNode>> cache_map;

        std::shared_ptr<CacheNode> head;
        std::shared_ptr<CacheNode> tail;

        size_t capacity_bytes;
        size_t current_size_bytes;
        size_t max_file_size_bytes; // Maximum size of a single file in the cache

        mutable std::shared_mutex cache_mutex;

        //Statistics;
        mutable size_t cache_hits;
        mutable size_t cache_misses;

    public:
        LRUFileCache(size_t capacity_mb = 100, size_t max_file_mb = 20)
        : capacity_bytes(capacity_mb * 1024 * 1024),
          current_size_bytes(0), max_file_size_bytes(max_file_mb * 1024 * 1024),
          cache_hits(0), cache_misses(0)
        {
            head = std::make_shared<CacheNode>("head", CachedFile("", ""));
            tail = std::make_shared<CacheNode>("tail", CachedFile("", ""));
            head->next = tail;
            tail->prev = head;

            std::cout << "[FileCache] LRU Cache initialized: "
                      << capacity_mb << " MB capacity, "
                      << max_file_mb << " MB max file size." << std::endl;
        }

        std::shared_ptr<CachedFile> get(const std::string& file_path)
        {
            std::unique_lock<std::shared_mutex> lock(cache_mutex);

            auto it = cache_map.find(file_path);
            if (it != cache_map.end())
            {
                auto node = it->second;
                move_to_front(node);
                cache_hits++;

                std::cout << "[FileCache] Cache hit for: " << file_path 
                          << " (" << node->file.size_bytes << " bytes)" << std::endl;          
                
                return std::make_shared<CachedFile>(it->second->file);
            }
             
            cache_misses++;
            std::cout << "[FileCache] Cache miss for: " << file_path << std::endl;
            return nullptr;
        }

        bool put(const std::string& file_path, const CachedFile& file_data)
        {
            std::unique_lock<std::shared_mutex> lock(cache_mutex);

            if(file_data.size_bytes > max_file_size_bytes)
            {
                std::cout << "[FileCache] File too large to cache: " << file_path 
                          << " (" << file_data.size_bytes << " bytes, max: " 
                          << max_file_size_bytes << " bytes)" << std::endl;
                return false;
            }

            auto it = cache_map.find(file_path);
            if (it != cache_map.end())
            {
                auto node = it->second;
                current_size_bytes -= node->file.size_bytes;
                node->file = file_data; // Update existing file data
                current_size_bytes += file_data.size_bytes;
                move_to_front(node);

                std::cout << "[FileCache] Updated cache for: " << file_path 
                          << " (" << file_data.size_bytes << " bytes)" << std::endl;
                return true;
            }

            // Make space if needed
            while(current_size_bytes + file_data.size_bytes > capacity_bytes && !cache_map.empty())
            {
                evict_lru();
            }

            // Create new node and add to front
            auto new_node = std::make_shared<CacheNode>(file_path, file_data);
            cache_map[file_path] = new_node;
            add_to_front(new_node);
            current_size_bytes += file_data.size_bytes;
            std::cout << "[FileCache] Added to cache: " << file_path 
                      << " (" << file_data.size_bytes << " bytes)" 
                      << " | Total: " << (current_size_bytes / 1024) << "KB" << std::endl;
            
            return true;
        }

        // Get cache statistics
        struct CacheStats
        {
            size_t entries;
            size_t size_bytes;
            size_t capacity_bytes;
            double hit_ratio;
            size_t hits;
            size_t misses;
        };

        CacheStats getStats() const{
            std::shared_lock<std::shared_mutex> lock(cache_mutex);
            
            double hit_ratio = 0;
            size_t total_requests = cache_hits + cache_misses;
            if (total_requests > 0)
            {
                hit_ratio = static_cast<double>(cache_hits) / total_requests;
            }

            return CacheStats{
                cache_map.size(),
                current_size_bytes,
                capacity_bytes,
                hit_ratio,
                cache_hits,
                cache_misses
            };
        }

        // Clear all cached files
        void clear() {
            std::unique_lock<std::shared_mutex> lock(cache_mutex);
            
            cache_map.clear();
            head->next = tail;
            tail->prev = head;
            current_size_bytes = 0;
            cache_hits = 0;
            cache_misses = 0;
            
            std::cout << "[FileCache] Cache cleared" << std::endl;
        }
        
        void print_cache_state() const {
            std::shared_lock<std::shared_mutex> lock(cache_mutex);
            
            std::cout << "[FileCache] Current state:" << std::endl;
            std::cout << "  Entries: " << cache_map.size() << std::endl;
            std::cout << "  Size: " << (current_size_bytes / 1024) << "KB / " 
                    << (capacity_bytes / 1024) << "KB" << std::endl;
            
            std::cout << "  Files (most recent first): ";
            auto current = head->next;
            while (current != tail) {
                std::cout << current->key << " ";
                current = current->next;
            }
            std::cout << std::endl;
    }

    private:
        void move_to_front(std::shared_ptr<CacheNode> node) {
            // Remove from current position
            node->prev->next = node->next;
            node->next->prev = node->prev;
            
            // Add to front
            add_to_front(node);
        }
        
        // Add new node to front
        void add_to_front(std::shared_ptr<CacheNode> node) {
            node->next = head->next;
            node->prev = head;
            head->next->prev = node;
            head->next = node;
        }
        
        void evict_lru() {
            auto lru_node = tail->prev;
            if (lru_node == head) {
                return; // Cache is empty
            }
            
            std::cout << "[FileCache] Evicting LRU: " << lru_node->key 
                    << " (" << lru_node->file.size_bytes << " bytes)" << std::endl;
            
            // Remove from map
            cache_map.erase(lru_node->key);
            
            // Remove from linked list
            lru_node->prev->next = lru_node->next;
            lru_node->next->prev = lru_node->prev;
            
            // Update size
            current_size_bytes -= lru_node->file.size_bytes;
        }
};

// Singleton instance
class FileCacheManager {
public:
    FileCacheManager(const FileCacheManager&) = delete;
    FileCacheManager& operator=(const FileCacheManager&) = delete;

    // Cho phép khởi tạo tùy biến 1 lần (nếu bạn thực sự cần)
    static LRUFileCache& get_instance(size_t capacity_mb = 100, size_t max_file_mb = 20) {
        static LRUFileCache instance(capacity_mb, max_file_mb);
        return instance;
    }
};

#endif // FILE_CACHE_H