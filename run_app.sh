#!/bin/bash

echo "Building Multiverse32 application..."

# Clean and build
cd build && make clean && cd ..
cmake -B build
make -C build

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Opening Multiverse32.app..."
    open build/Win32HelloWorld.app
else
    echo "Build failed!"
    exit 1
fi
