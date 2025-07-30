#!/bin/bash

# Simple build script
echo "Building web server..."

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make

echo "Done! Run with: ./webserver"