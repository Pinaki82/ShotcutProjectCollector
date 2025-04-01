# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name and language
project(ShotcutProjectCollector LANGUAGES C)

# Set C standard to C99
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define source files
set(SOURCES
    src/main.c
    src/parser.c
    src/file_utils.c
    src/logging.c
)

# Add executable target
add_executable(shotcut_project_collector ${SOURCES})

# Define include directories for the target
target_include_directories(shotcut_project_collector PRIVATE src)

# Set Debug and Release compiler flags
add_definitions(-DDEBUG) # Uncomment this line for additional debugging macros
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra -pedantic -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")

# Optionally, enable position-independent code (PIC) if needed
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Link any necessary libraries (if applicable)
# target_link_libraries(shotcut_project_collector <library_name>)

# Display the current build type
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Install the executable to the bin directory
install(TARGETS shotcut_project_collector DESTINATION bin)
