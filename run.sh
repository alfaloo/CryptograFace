#!/bin/bash

# Check if a file name was provided as an argument
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <cpp_file_name>"
    exit 1
fi

# Extract the base name for the executable
file_name="$1"
executable="${file_name%.*}"

# Directory where executables will be stored
build_dir="./builds"

# Create the builds directory if it doesn't exist
if [ ! -d "$build_dir" ]; then
    mkdir "$build_dir"
fi

# Compile the C++ file with g++ and OpenCV
g++ -std=c++11 "$file_name" -o "$build_dir/$executable" `pkg-config --cflags --libs opencv4`
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# Run the executable
./"$build_dir/$executable"