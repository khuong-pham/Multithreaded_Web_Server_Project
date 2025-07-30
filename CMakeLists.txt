add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_connection test_integration
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests (unit + integration)"
)
cmake_minimum_required(VERSION 3.10)
project(webserver)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(src/core)
include_directories(src/http)
include_directories(src/handlers)
include_directories(src/threading)
include_directories(src/connection)
include_directories(src/cache)

file(GLOB_RECURSE HEADERS
    src/core/*.h
    src/http/*.h
    src/handlers/*.h
    src/threading/*.h
    src/connection/*.h
    src/cache/*.h
)

# All source files
add_executable(webserver
    src/core/main.cpp
    src/core/Server.cpp
    src/http/HttpRequest.cpp
    src/http/HttpParser.cpp
    src/handlers/FileHandler.cpp
    src/handlers/ResponseGenerator.cpp
    src/threading/ThreadPool.cpp
    src/connection/Connection.cpp
)
 
# Link pthread
find_package(Threads REQUIRED) 
target_link_libraries(webserver Threads::Threads)

# Install binary
install(TARGETS webserver
    RUNTIME DESTINATION bin
)

# Copy public static files to build directory for testing/dev
add_custom_command(TARGET webserver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/public $<TARGET_FILE_DIR:webserver>/public
)


# GoogleTest unit test
find_package(GTest REQUIRED)
if(GTest_FOUND)
    enable_testing()

    function(add_gtest name)
        add_executable(${name} ${ARGN})
        target_include_directories(${name} PRIVATE ${GTEST_INCLUDE_DIRS} src/)
        target_link_libraries(${name} GTest::GTest GTest::Main Threads::Threads)
        add_test(NAME ${name} COMMAND ${name})
    endfunction()

    add_gtest(unit_tests
        tests/unit_tests.cpp
        src/core/Server.cpp
        src/connection/Connection.cpp
        src/http/HttpParser.cpp
        src/http/HttpRequest.cpp
        src/handlers/ResponseGenerator.cpp
        src/handlers/FileHandler.cpp
        src/threading/ThreadPool.cpp
    )

    add_gtest(test_integration
        tests/integration_tests.cpp
    )

    add_gtest(load_tests
        tests/load_tests.cpp
    )
endif()