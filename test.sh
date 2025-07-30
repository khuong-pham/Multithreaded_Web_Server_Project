#!/bin/bash

echo "🏗️ Building and running tests..."

# Step 1: Build project and navigate into build directory
./build.sh
cd build

# Step 2: Prepare log files and default test result flags
TEST_LOG="tests.log"
rm -f "$TEST_LOG"
UNIT_RESULT=1
INTEG_RESULT=1
LOAD_RESULT=1

# Utility: Get current timestamp in formatted style
timestamp() {
    date '+[%Y-%m-%d %H:%M:%S]'
}

# Utility: Prefix each line of input with a timestamp
add_timestamp() {
    while IFS= read -r line; do
        echo "$(timestamp) $line"
    done
}

# Step 3: Run unit tests if binary is available
if [ -f "unit_tests" ]; then
    echo "🧪 Running unit tests... (log: $TEST_LOG)"
    {
        echo "--- UNIT TESTS ---"
        ./unit_tests
    } 2>&1 | add_timestamp >> "$TEST_LOG"
    
    ./unit_tests > /dev/null 2>&1
    UNIT_RESULT=$?
    
    if [ $UNIT_RESULT -eq 0 ]; then
        echo "✅ Unit tests PASSED!"
    else
        echo "❌ Unit tests FAILED! (see $TEST_LOG)"
    fi
    echo ""
else
    echo "⚠️  unit_tests binary not found, skip unit tests."
    echo ""
fi

# Step 4: Terminate any leftover server instances and free up port 8080
echo "🧹 Killing any old webserver process..."
pkill -9 -f ./webserver 2>/dev/null || true
# Wait for port 8080 to be free (max 10s)
for i in {1..10}; do
    if ! lsof -i :8080 >/dev/null 2>&1; then
        break
    fi
    echo "⏳ Waiting for port 8080 to be free... ($i)"
    sleep 1
done

# Step 5: Start web server in background with log timestamps
echo "🚀 Starting server (run in background, log: server.log)..."
setsid ./webserver 2>&1 | add_timestamp > server_test.log &
SERVER_PID=$!
sleep 2

# Verify that the server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "❌ Server failed to start!"
    exit 1
fi

# Step 6: Run integration tests if binary is available
if [ -f "test_integration" ]; then
    echo "🧪 Running integration tests... (log: $TEST_LOG)"
    {
        echo "--- INTEGRATION TESTS ---"
        ./test_integration
    } 2>&1 | add_timestamp >> "$TEST_LOG"
    
    # Get exit code directly
    ./test_integration > /dev/null 2>&1
    INTEG_RESULT=$?
    
    if [ $INTEG_RESULT -eq 0 ]; then
        echo "✅ Integration tests PASSED!"
    else
        echo "❌ Integration tests FAILED! (see $TEST_LOG)"
    fi
    echo ""
else
    echo "⚠️  test_integration binary not found, skip integration tests."
    echo ""
fi

# Step 7: Run load tests if binary is available
if [ -f "load_tests" ]; then
    echo "🧪 Running load tests... (log: $TEST_LOG)"
    {
        echo "--- LOAD TESTS ---"
        ./load_tests
    } 2>&1 | add_timestamp >> "$TEST_LOG"
    
    # Get exit code directly
    ./load_tests > /dev/null 2>&1
    LOAD_RESULT=$?
    
    if [ $LOAD_RESULT -eq 0 ]; then
        echo "✅ Load tests PASSED!"
    else
        echo "❌ Load tests FAILED! (see $TEST_LOG)"
    fi
    echo ""
else
    echo "⚠️  load_tests binary not found, skip load tests."
    echo ""
fi

# Step 7.5: Run performance benchmarking (NEW ADDITION)
echo "📊 Running performance benchmarks..."
PERF_LOG="performance_benchmark.log"
PERF_RESULT=1

{
    echo "--- PERFORMANCE BENCHMARKS ---"
    echo "Starting comprehensive performance testing..."
    
    # Check if server is responding
    if curl -s http://localhost:8080/ > /dev/null; then
        echo "Server is responding, starting benchmarks..."
        
        # Create CSV header
        echo "Concurrency,RPS,P50,P95,P99,Transfer_Rate" > performance_results.csv
        
        # Run benchmark across different concurrency levels
        for CONCURRENCY in 1 2 5 10 15 20 25 30; do
            echo "Testing concurrency: $CONCURRENCY"
            
            START_TIME=$(date +%s)
            RESULT=$(ab -n 1000 -c $CONCURRENCY http://localhost:8080/ 2>/dev/null)
            
            if [ $? -eq 0 ]; then
                RPS=$(echo "$RESULT" | grep "Requests per second" | awk '{print $4}')
                P50=$(echo "$RESULT" | grep "50%" | awk '{print $2}')
                P95=$(echo "$RESULT" | grep "95%" | awk '{print $2}')
                P99=$(echo "$RESULT" | grep "99%" | awk '{print $2}')
                TRANSFER=$(echo "$RESULT" | grep "Transfer rate" | awk '{print $3}')
                
                echo "$CONCURRENCY,$RPS,$P50,$P95,$P99,$TRANSFER" >> performance_results.csv
                
                END_TIME=$(date +%s)
                DURATION=$((END_TIME - START_TIME))
                echo "  Results: RPS=$RPS, P50=${P50}ms, P95=${P95}ms, P99=${P99}ms (${DURATION}s)"
            else
                echo "  ERROR: Test failed for concurrency $CONCURRENCY"
                echo "$CONCURRENCY,ERROR,ERROR,ERROR,ERROR,ERROR" >> performance_results.csv
            fi
            
            sleep 1
        done
        
        # Find optimal performance point
        BEST_RPS=$(tail -n +2 performance_results.csv | grep -v ERROR | sort -t, -k2 -nr | head -1)
        if [ ! -z "$BEST_RPS" ]; then
            echo "Best performance: $BEST_RPS"
            PERF_RESULT=0
        fi
        
        echo "Performance results saved to performance_results.csv"
    else
        echo "ERROR: Server not responding, skipping performance tests"
    fi
    
} 2>&1 | add_timestamp >> "$PERF_LOG"

if [ $PERF_RESULT -eq 0 ]; then
    echo "✅ Performance benchmarks COMPLETED!"
    # Display quick summary
    if [ -f "performance_results.csv" ]; then
        BEST_LINE=$(tail -n +2 performance_results.csv | grep -v ERROR | sort -t, -k2 -nr | head -1)
        if [ ! -z "$BEST_LINE" ]; then
            BEST_CONCURRENCY=$(echo $BEST_LINE | cut -d, -f1)
            BEST_RPS=$(echo $BEST_LINE | cut -d, -f2)
            BEST_P95=$(echo $BEST_LINE | cut -d, -f4)
            echo "🚀 Peak performance: ${BEST_RPS} RPS at ${BEST_CONCURRENCY} concurrent (P95: ${BEST_P95}ms)"
        fi
    fi
else
    echo "❌ Performance benchmarks FAILED! (see $PERF_LOG)"
fi
echo ""

if [ $PERF_RESULT -eq 0 ]; then
    echo "✅ Performance benchmarks COMPLETED!"
    # Display quick summary
    if [ -f "performance_results.csv" ]; then
        BEST_LINE=$(tail -n +2 performance_results.csv | grep -v ERROR | sort -t, -k2 -nr | head -1)
        if [ ! -z "$BEST_LINE" ]; then
            BEST_CONCURRENCY=$(echo $BEST_LINE | cut -d, -f1)
            BEST_RPS=$(echo $BEST_LINE | cut -d, -f2)
            BEST_P95=$(echo $BEST_LINE | cut -d, -f4)
            echo "🚀 Peak performance: ${BEST_RPS} RPS at ${BEST_CONCURRENCY} concurrent (P95: ${BEST_P95}ms)"
        fi
    fi
else
    echo "❌ Performance benchmarks FAILED! (see $PERF_LOG)"
fi
echo ""

# Step 8: Display overall test results summary
echo ""
echo "========== TEST SUMMARY =========="
[ $UNIT_RESULT -eq 0 ] && echo "Unit tests:        ✅ PASSED" || echo "Unit tests:        ❌ FAILED"
[ $INTEG_RESULT -eq 0 ] && echo "Integration tests: ✅ PASSED" || echo "Integration tests: ❌ FAILED"
[ $LOAD_RESULT -eq 0 ] && echo "Load tests:        ✅ PASSED" || echo "Load tests:        ❌ FAILED"
[ $PERF_RESULT -eq 0 ] && echo "Performance tests: ✅ PASSED" || echo "Performance tests: ❌ FAILED"
echo "=================================="

# Step 9: Terminate the web server process group
echo "🛑 Stopping server..."
if kill -0 $SERVER_PID 2>/dev/null; then
    kill -TERM -$SERVER_PID 2>/dev/null
    # Wait up to 5 seconds for shutdown
    for i in {1..5}; do
        if ! kill -0 $SERVER_PID 2>/dev/null; then
            echo "✅ Server stopped gracefully"
            break
        fi
        sleep 1
    done
    # Force kill if still running
    if kill -0 $SERVER_PID 2>/dev/null; then
        echo "⚠️  Force killing server group..."
        kill -KILL -$SERVER_PID 2>/dev/null
        sleep 1
    fi
fi

# Step 10: Final cleanup to ensure no remaining server processes or port blockage
echo "🧹 Final cleanup: killing any remaining webserver..."
pkill -9 -f ./webserver 2>/dev/null || true
for i in {1..10}; do
    if ! lsof -i :8080 >/dev/null 2>&1; then
        break
    fi
    echo "⏳ Waiting for port 8080 to be free after tests... ($i)"
    sleep 1
done

echo "✅ All tests complete!"

# Step 11: Return final result for manual review
if [ $UNIT_RESULT -eq 0 ] && [ $INTEG_RESULT -eq 0 ] && [ $LOAD_RESULT -eq 0 ]; then
    echo "🎉 ALL TESTS PASSED!"
    exit 0
else
    echo "💥 SOME TESTS FAILED!"
    exit 1
fi