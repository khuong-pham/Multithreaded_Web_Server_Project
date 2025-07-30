# Multithreaded HTTP Server

A high-performance HTTP server implementation in C++17 featuring thread pool architecture, intelligent connection management, LRU file caching, and comprehensive performance optimization. Built from scratch to demonstrate advanced systems programming concepts.

## 🚀 Performance Highlights

**Latest Benchmark Results (2025):**
- **Peak Performance**: 77.02 RPS at 15 concurrent connections
- **Linear Scaling**: 229% performance improvement (23.39 → 77.02 RPS)
- **Optimal Range**: 10-20 concurrent connections
- **Hardware**: 6 cores / 12 logical threads

## 📊 Performance Benchmarks

### Performance Results Summary

| Concurrency | RPS      | P50 (ms) | P90 (ms) | P99 (ms) | Performance Zone      |
|-------------|----------|----------|----------|----------|-----------------------|
| 1           | **23.39** | 26.62    | 43.89    | 61.58    | ✅ Baseline            |
| 2           | **41.28** | 25.99    | 47.72    | 75.19    | ✅ Linear Scaling      |
| 4           | **54.48** | 49.95    | 90.50    | 137.69   | ✅ Linear Scaling      |
| 8           | **60.41** | 107.29   | 175.35   | 242.15   | ✅ Linear Scaling      |
| 10          | **67.52** | 118.47   | 192.80   | 441.36   | ✅ Linear Scaling      |
| **15**      | **77.02** | **166.66** | **281.40** | **417.16** | 🔥 **Peak Performance** |
| 20          | **76.80** | 205.65   | 370.14   | 681.82   | ⚠️ Thread Saturation   |
| 25          | **43.44** | 314.56   | 635.35   | 1270.00  | 🔴 Overload            |
| 30          | **27.00** | 833.28   | 1320.00  | 1730.00  | 🔴 Severe Overload     |

### Performance Zone Analysis

- **Linear Scaling Zone (1-15 connections)**: Consistent 229% performance improvement with predictable latency growth
- **Performance Plateau (15-20 connections)**: Peak throughput achieved with increasing latency 
- **Degradation Zone (25+ connections)**: Server overload with resource contention and thread starvation

## 🏗️ Architecture Overview

### System Design

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Client        │───▶│   Main Thread   │───▶│   Thread Pool   │
│   Requests      │    │   (Accept Loop) │    │   (12 Workers)  │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                │                       │
                                ▼                       ▼
                       ┌─────────────────┐    ┌─────────────────┐
                       │   Connection    │    │   LRU Cache     │
                       │   Manager       │    │   (100MB)       │
                       └─────────────────┘    └─────────────────┘
```

### Core Components

#### 1. Thread Pool Architecture

**Configuration:**
- **Worker Threads**: 12 threads (based on `std::thread::hardware_concurrency()`)
- **Task Queue**: Producer-consumer pattern with condition variables
- **Load Balancing**: Automatic task distribution across available workers

**Thread Pool Benefits:**
- Eliminates thread creation overhead
- Bounded resource usage with fixed thread count
- Optimal CPU utilization across available cores

#### 2. Intelligent Keep-Alive Management

**Decision Logic:**
The server employs a three-factor decision algorithm for connection management:

- **Client Preference**: Analyzes `Connection: keep-alive` header
- **Server Capacity**: Monitors request count vs. configured limits
- **Traffic Control**: Evaluates active connections vs. threshold

**Connection Termination Triggers:**
- Client sends `Connection: close` header
- Maximum request limit reached per connection
- Connection timeout exceeded
- Network errors or malformed requests
- Server processing exceptions

**Connection Lifecycle:**
Each worker thread manages the complete connection lifecycle through five phases:

1. **Reading Phase**: Parse HTTP request 
2. **Processing Phase**: Route request and generate appropriate response  
3. **Writing Phase**: Send response with proper HTTP headers
4. **Decision Phase**: Evaluate keep-alive conditions 
5. **Cleanup Phase**: Resource deallocation and connection closure

#### 3. **LRU File Caching System**
Thread-safe, high-performance file caching with LRU eviction:

**Technical Specifications:**
- **Total Capacity**: 100MB memory allocation
- **Maximum File Size**: 20MB per cached file
- **Thread Safety**: `shared_mutex` enabling concurrent read operations
- **Access Complexity**: O(1) lookup using hash map + doubly linked list
- **Eviction Policy**: Least Recently Used (LRU) automatic memory management

**Performance Impact:**
- Eliminates repeated disk I/O operations
- Reduces file system access latency
- Improves overall server throughput

## 🔄 Development Journey

### Phase 1: Foundation
**Objective**: Create basic TCP server infrastructure

**Implementation:**
- Basic TCP socket implementation with POSIX sockets
- Single-threaded request handling  
- Foundation for HTTP protocol support

### Phase 2: Static File Serving
**Objective**: Implement file serving capabilities

**Implementation:**
- FileHandler class for parsing and erving requests.
- MIME type detection for proper Content-Type headers
- Path traversal security protection mechanisms
- Basic HTTP response generation

### Phase 3: Threading & Concurrency
**Objective**: Enable concurrent request processing

**Implementation:**
- ThreadPool class with worker thread management
- Thread-safe task queue using condition variables
- Producer-consumer pattern for task distribution
- Concurrent processing of multiple client connections

### Phase 4: Connection Management
**Objective**: Implement HTTP keep-alive functionality

**Implementation:**
- Keep-alive connection lifecycle management
- Traffic control and resource management policies
- Connection timeout and error handling mechanisms
- Intelligent connection reuse strategies

### Phase 5: Performance Optimization
**Objective**: Implement caching for performance enhancement

**Implementation:**
- In-memory LRU file caching system
- Thread-safe cache operations with shared_mutex
- Significant performance improvements and reduced I/O
- Memory-efficient cache management

### Phase 6: Testing & Quality Assurance
**Objective**: Comprehensive testing framework

**Implementation:**
- Integration of Google Test framework
- Comprehensive unit testing coverage
- Integration testing suite for system validation
- Performance benchmarking and analysis tools


## 🔧 Worker Thread Task Handling

### Producer-Consumer Pattern
The server implements a task distribution system using the producer-consumer pattern with thread-safe synchronization.

#### Task Processing Flow

Each worker thread follows a structured execution pattern:

1. **Infinite Loop**: Worker thread runs until stop signal received
2. **Task Waiting**: Waits on condition variable for new tasks or stop signal
3. **Stop Check**: Exits if `stop_flag` set and queue empty
4. **Task Extraction**: Removes task from queue, updates counters
5. **Task Execution**: Increments `active_threads`, executes task, increments `total_tasks_processed`
6. **Notification**: Decrements `active_threads`, notifies other threads via `condition.notify_all()`

### Synchronization Features
- **Mutex Protection**: `Mutex` ensures atomic queue operations
- **Condition Variables**: Efficient thread blocking and notification
- **Atomic Counters**: Thread-safe statistics tracking

## ⚖️ Architecture Analysis

### ✅ Advantages

#### **Good Scalability**
- **Linear Performance Scaling**: Up to hardware thread limits
- **Optimal CPU Utilization**: Hardware-based thread pool sizing
- **Intelligent Load Distribution**: Automatic task balancing across workers

#### **Resource Management**
- **Bounded Memory Usage**: Configurable limits prevent resource exhaustion
- **I/O Optimization**: LRU caching eliminates repeated disk operations
- **Connection Efficiency**: Keep-alive reduces TCP handshake overhead

#### **Production Readiness**
- **Thread-Safe Operations**: Comprehensive synchronization throughout codebase
- **Error Handling**: Error handling implementation
- **Security Measures**: Path traversal protection and input validation

#### **Performance Optimizations**
- **Efficient File Serving**: Minimized copy operations where possible
- **Smart Connection Management**: Intelligent keep-alive decision logic
- **Configurable Parameters**: Adjustable timeouts and resource limits

### ❌ Limitations

#### **Concurrency Ceiling**
- **Hardware Bound**: Limited by thread pool size (12 threads)
- **Memory Overhead**: Each thread consumes ~8MB stack space
- **Context Switching**: Performance degradation beyond optimal load

#### **I/O Model Constraints**
- **Blocking Operations**: Threads blocked during file I/O
- **Cache Misses**: Uncached files still require disk access
- **Network Latency**: Synchronous socket operations

#### **Scalability Bottlenecks**
- **Connection Limits**: Artificial limits to prevent overload
- **Memory Usage**: Cache size affects maximum file set

## 📁 Project Structure

```
multithreaded-server/
├── src/                         # Source code
│   ├── cache/                   # LRU caching implementation
│   ├── connection/             # Connection management
│   ├── core/                   # Core server logic
│   ├── handlers/               # Request handlers
│   ├── http/                   # HTTP protocol implementation
│   ├── public/                 # Static files
│   └── threading/              # Thread pool implementation
├── tests/                      # Testing implementation
├── build.sh                    # Build automation
├── test.sh                     # Test runner
├── Benchmark_report.md         # Performance analysis
└── README.md                   # This file
```

## 🛠️ Building & Running

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake libgtest-dev

```

### Build Instructions
```bash
# Clone the repository
git clone https://github.com/khuong-pham/Multithreaded_Web_Server_Project.git
cd Multithreaded_Web_Server

# Build the project
chmod +x build.sh
./build.sh

# The build script will:
# 1. Create build directory
# 2. Run CMake configuration
# 3. Compile all targets
# 4. Run basic validation tests
```

### Running the Server
```bash
# Start the server (default port 8080)
cd build
./webserver your_server_port(optional)

# Server will start with output:
# [Server] Initializing server on port 8080
# [Server] Thread pool initialized with 12 threads
# [Server] Server started successfully on http://localhost:8080

# Access the server
curl http://localhost:8080/
# or visit in browser: http://localhost:8080
```

### Individual Test Suites
```bash
# Unit tests only
cd build
./unit_tests

# Integration tests
./integration_tests

# Performance testing
./load_tests 

# Custom load testing
./load_tests 
```

### Performance Benchmarking
```bash
# You can read comprehensive peformance benchmark result through Benchmark_report.md
```

### Manual Testing Tools
```bash
# Apache Bench testing
ab -n 10000 -c 15 http://localhost:8080/

# wrk testing (if available)
wrk -t4 -c15 -d30s http://localhost:8080/

```

## 📈 Performance Optimization

### Current Optimizations
1. **LRU File Caching**: Eliminates repeated disk I/O
2. **Keep-Alive Connections**: Reduces TCP handshake overhead  
3. **Thread Pool**: Optimal CPU utilization
4. **Traffic Control**: Prevents resource exhaustion

### Future Enhancements
1. **Event-Driven I/O**: epoll/kqueue for better scalability
2. **HTTP/2 Support**: Multiplexing and server push
3. **Compression**: Gzip/Brotli response compression
4. **SSL/TLS**: Secure connection support

## 🙋‍♂️ Author

**Systems Programming Demonstration Project**
- **Purpose**: Educational implementation showcasing advanced C++ systems programming
- **Focus**: High-performance server architecture and optimization techniques
- **Technologies**: Modern C++17, POSIX Sockets, Threading, Memory Management

## 🙏 Acknowledgments

- Built from scratch to demonstrate systems programming ability
- The overall architecture takes inspiration from real-world high-performance servers.
- Performance testing method based on industry standards
- Uses thread-safe design patterns, following modern C++ best practices.

---

⭐ **Star this repository if you find it useful for learning systems programming!**
