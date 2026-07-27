cmake_minimum_required(VERSION 3.10) 

# Prevent building in the source directory
# This keeps our source tree clean from build artifacts
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "make a different build directory in the project and use cmake.. in build directory")
endif()


# Project Name 
project(cfs_schedular)



# This gives us modern features like smart pointers, lambdas, auto, etc.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)


# Add compiler flags for better code quality
# -Wall: Enable all common warnings
# -Wextra: Enable extra warnings
# -pthread: Enable pthread support for threading
add_compile_options(-Wall -Wextra -pthread)


# Finding the threading library which is required for Real-Time Monitoring
find_package(Threads REQUIRED)


# Add include directories for header files
# ${CMAKE_CURRENT_SOURCE_DIR}/src: Our source headers
# ${CMAKE_CURRENT_SOURCE_DIR}/include: External headers (nlohmann/json.hpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)



# Collect all C++ source files recursively from src/ directory
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"    # All .cpp files in src/ and subdirectories
    "main.cpp"     # Main entry point
)

# Creating the main executable
add_executable(cfs_scheduler ${SOURCES})


# Link with threading library for real time monitoring
target_link_libraries(cfs_scheduler Threads::Threads)