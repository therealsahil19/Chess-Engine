cmake_minimum_required(VERSION 3.20)
project(ChessEngineNative)

set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0077 NEW)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

# --- Raylib Integration ---
add_subdirectory(src/libs/raylib-5.0)

# --- Sources ---
file(GLOB CORE_SOURCES "src/core/*.cpp")
file(GLOB GUI_SOURCES "src/gui/*.cpp")

# --- Executable ---
add_executable(ChessApp src/main.cpp ${CORE_SOURCES} ${GUI_SOURCES})
target_link_libraries(ChessApp PRIVATE raylib)

# Copy assets to build directory if needed (or just reference them)
# file(COPY src/assets DESTINATION ${CMAKE_BINARY_DIR}/assets)
