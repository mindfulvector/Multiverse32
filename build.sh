#!/bin/bash

# Cross-platform build script
set -e

PLATFORM=""
BUILD_TYPE="Release"
GENERATOR=""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --platform)
            PLATFORM="$2"
            shift 2
            ;;
        --type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --generator)
            GENERATOR="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Detect platform if not specified
if [ -z "$PLATFORM" ]; then
    case "$(uname -s)" in
        Darwin)
            PLATFORM="macos"
            ;;
        MINGW*|CYGWIN*|MSYS*)
            PLATFORM="windows"
            ;;
        Linux)
            PLATFORM="linux"
            ;;
        *)
            echo "Unsupported platform"
            exit 1
            ;;
    esac
fi

echo "Building for platform: $PLATFORM"
echo "Build type: $BUILD_TYPE"

# Create build directory
BUILD_DIR="build-$PLATFORM"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure based on platform
case "$PLATFORM" in
    windows)
        if [ -z "$GENERATOR" ]; then
            if command -v cl >/dev/null 2>&1; then
                GENERATOR="Visual Studio 16 2019"
            else
                GENERATOR="MinGW Makefiles"
            fi
        fi
        cmake .. -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        ;;
    macos)
        if [ -z "$GENERATOR" ]; then
            GENERATOR="Unix Makefiles"
        fi
        cmake .. -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        ;;
    ios-device)
        cmake .. \
            -G Xcode \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake \
            -DPLATFORM=OS64 \
            -DCMAKE_OSX_ARCHITECTURES=arm64
        ;;
    ios-simulator)
        cmake .. \
            -G Xcode \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake \
            -DPLATFORM=SIMULATOR64 \
            -DCMAKE_OSX_ARCHITECTURES=x86_64
        ;;
    linux)
        cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        ;;
    *)
        echo "Unsupported platform: $PLATFORM"