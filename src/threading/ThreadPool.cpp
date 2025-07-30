#include "ThreadPool.h"
#include <chrono>

ThreadPool::ThreadPool(size_t num_threads) : stop_flag(false), active_threads(0), current_queue_size(0), total_tasks_processed(0), pool_size(num_threads) {
    std::cout << "[ThreadPool] Initializing with " << pool_size << " threads..." << std::endl;
   
    // Create worker threads
    workers.reserve(num_threads);
    for (size_t i = 0; i < num_threads; i++) {
        workers.emplace_back([this, i] {
            std::cout << "[ThreadPool] Worker thread " << i << " started" << std::endl;
            workerLoop();
            std::cout << "[ThreadPool] Worker thread " << i << " finished" << std::endl;
        });
    }

    std::cout << "[ThreadPool] All worker threads initialized successfully" << std::endl;
}

ThreadPool::~ThreadPool() {
    std::cout << "[ThreadPool] Destructor called, Shutting down..." << std::endl;
    shutdown();
}

void ThreadPool::shutdown()
{
    std::cout << "[ThreadPool] Shutting down called" << std::endl;

    stop_flag.store(true);

    condition.notify_all();  // Wake up all threads to finish their work

    // Wait for all threads to finish
    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::cout << "[ThreadPool] All threads shut down. Total tasks processed: " 
              << total_tasks_processed.load() << std::endl;
}

void ThreadPool::waitForAllTasks() {
    std::unique_lock<std::mutex> lock(queue_mutex);

    // Wait until all tasks are processed and no tasks are left in the queue
    condition.wait(lock, [this] {
        return tasks.empty() && active_threads.load() == 0;
    });

    std::cout << "[ThreadPool] All tasks completed. " << std::endl;
}

void ThreadPool::workerLoop()
{
    while(!stop_flag.load())
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] {
                return stop_flag.load() || !tasks.empty();
            });

            if (stop_flag.load() && tasks.empty()) {
                return; // Exit if stopping and no tasks left
            }

            if(!tasks.empty())
            {
                task = std::move(tasks.front());
                tasks.pop();
                current_queue_size.store(tasks.size());
            }
        }

        // Execute task if we got one
        if(task)
        {
            try
            {
                active_threads.fetch_add(1);

                task();

                total_tasks_processed.fetch_add(1);
            }
            catch(const std::exception& e)
            {
                std::cerr << "[ThreadPool] Task execution failed: " << e.what() << std::endl;
            }
            catch(...)
            {
                 std::cerr << "[ThreadPool] Task execution failed with unknown exception" << std::endl;
            }

            active_threads.fetch_sub(1);

            condition.notify_all(); // Notify that a task has been completed
        }
    }
}

void ThreadPool::printStatus() const {
    std::cout << "\n=== ThreadPool Status ===" << std::endl;
    std::cout << "Pool Size: " << pool_size << std::endl;
    std::cout << "Active Threads: " << active_threads.load() << std::endl;
    std::cout << "Queue Size: " << current_queue_size.load() << std::endl;
    std::cout << "Total Tasks Processed: " << total_tasks_processed.load() << std::endl;
    std::cout << "Status: " << (stop_flag.load() ? "STOPPED" : "RUNNING") << std::endl;
    std::cout << "=========================" << std::endl;
}