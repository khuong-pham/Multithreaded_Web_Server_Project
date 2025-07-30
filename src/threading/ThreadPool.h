#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <iostream>

class ThreadPool {
    private:
        // Thread managenent
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;

        // Synchronization
        std::mutex queue_mutex;
        std::condition_variable condition;

        // Pool state
        std::atomic<bool> stop_flag;
        std::atomic<size_t> active_threads;

        // Statistics
        std::atomic<size_t> total_tasks_processed;
        std::atomic<size_t> current_queue_size;

        // Poll size
        size_t pool_size;

    public:
        ThreadPool(size_t num_threads = std::thread::hardware_concurrency());
        ~ThreadPool();

        // Delete copy constructor and copy assignment operators
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        /**
         * @brief Enqueue a task into the thread pool for asynchronous execution.
         *
         * This function accepts any callable (function, lambda, functor) and its arguments,
         * It returns a std::future<T> that can be used to retrieve the result of the task,
         * where T is the return type of the callable.
         *
         * The task is executed in the background, and the calling thread can continue
         * execution without blocking.
         *
         * @tparam F    Type of the callable to be executed.
         * @tparam Args Types of the arguments to be passed to the callable.
         * @param f     The callable object (function, lambda, etc.).
         * @param args  Arguments to pass to the callable.
         * @return std::future<T> representing the result of the task.
         *
         * @note This method uses perfect forwarding to preserve the value categories
         *       of the passed arguments.
         */
        template<class F, class... Args>
        auto enqueue(F&& f, Args&&... args) 
            -> std::future<typename std::result_of<F(Args...)>::type>;

        // Pool management
        void shutdown();
        void waitForAllTasks();

        // Statistics and monitoring
        size_t getActiveThreads() const { return active_threads.load(); }
        size_t getQueueSize() const { return current_queue_size.load(); }
        size_t getTotalTasksProcessed() const { return total_tasks_processed.load(); }
        size_t getPoolSize() const { return pool_size; }
        bool isStopped() const { return stop_flag.load(); }

        // Pool status
        void printStatus() const;

    private:
        // Worker thread function
        void workerLoop();
};

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> 
        {
            using return_type = typename std::result_of<F(Args...)>::type;
            
            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
            
            std::future<return_type> result = task->get_future();
            
            // Add task to queue
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                
                // Don't allow enqueueing after stopping the pool
                if (stop_flag.load()) {
                    throw std::runtime_error("Cannot enqueue task: ThreadPool is stopped");
                }
                
                tasks.emplace([task](){ (*task)(); });
                current_queue_size.store(tasks.size());
            }
            
            condition.notify_one();
    
            return result;  
        }
#endif // THREADPOOL_H