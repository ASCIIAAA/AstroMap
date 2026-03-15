cmake_minimum_required(VERSION 3.10)
project(AstroMap)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include)

# Source files
add_executable(AstroMap 
    main.cpp 
    src/Octree.cpp
    # Add other .cpp files as you build them
)