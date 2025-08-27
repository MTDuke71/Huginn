#!/bin/bash

# Huginn Cross-Platform Build Script for Linux/macOS
# This script helps configure and build with different toolchains

set -e

echo "===================================="
echo "Huginn Cross-Platform Build Script"
echo "===================================="
echo

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "Error: CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Show available options
echo "Available build configurations:"
echo "1. GCC Debug"
echo "2. GCC Release"
echo "3. Clang Debug (if available)"
echo "4. Clang Release (if available)"
echo "5. Clean all build directories"
echo "6. Exit"
echo

read -p "Enter your choice (1-6): " choice

case $choice in
    1)
        preset="gcc-x64-debug"
        action="build"
        ;;
    2)
        preset="gcc-x64-release"
        action="build"
        ;;
    3)
        # Check if clang is available
        if command -v clang++ &> /dev/null; then
            echo "Building with Clang Debug..."
            mkdir -p build/clang-x64-debug
            cd build/clang-x64-debug
            cmake ../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ -G Ninja
            action="custom"
        else
            echo "Clang not found. Please install clang or choose a different option."
            exit 1
        fi
        ;;
    4)
        # Check if clang is available
        if command -v clang++ &> /dev/null; then
            echo "Building with Clang Release..."
            mkdir -p build/clang-x64-release
            cd build/clang-x64-release
            cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -G Ninja
            action="custom"
        else
            echo "Clang not found. Please install clang or choose a different option."
            exit 1
        fi
        ;;
    5)
        action="clean"
        ;;
    6)
        echo "Exiting..."
        exit 0
        ;;
    *)
        echo "Invalid choice. Exiting..."
        exit 1
        ;;
esac

if [ "$action" = "clean" ]; then
    echo "Cleaning all build directories..."
    if [ -d "build" ]; then
        rm -rf build
        echo "Build directories cleaned."
    else
        echo "No build directories found."
    fi
    exit 0
fi

if [ "$action" = "custom" ]; then
    echo
    echo "Building..."
    ninja
    
    if [ $? -eq 0 ]; then
        echo
        echo "Build completed successfully!"
        echo
        
        read -p "Do you want to run tests? (y/n): " run_tests
        if [[ $run_tests =~ ^[Yy]$ ]]; then
            echo "Running tests..."
            ctest --output-on-failure
        fi
    else
        echo "Build failed!"
        exit 1
    fi
else
    echo
    echo "Configuring with preset: $preset"
    echo
    
    # Configure
    cmake --preset $preset
    
    echo
    echo "Building..."
    echo
    
    # Build
    cmake --build --preset $preset
    
    echo
    echo "Build completed successfully!"
    echo
    
    # Ask if user wants to run tests
    read -p "Do you want to run tests? (y/n): " run_tests
    if [[ $run_tests =~ ^[Yy]$ ]]; then
        echo "Running tests..."
        ctest --preset $preset
    fi
fi

echo
echo "Done!"
