cmake_minimum_required(VERSION 3.20)
project(ChessEngineNative)

set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0077 NEW)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(raylib)

# --- Sources ---
file(GLOB CORE_SOURCES "src/core/*.cpp")
file(GLOB ENGINE_SOURCES "src/engine/*.cpp")
file(GLOB GUI_SOURCES "src/gui/*.cpp")
message(STATUS "CORE SOURCES: ${CORE_SOURCES}")

# --- Executable ---
add_executable(ChessApp src/main.cpp ${CORE_SOURCES} ${ENGINE_SOURCES} ${GUI_SOURCES})
target_link_libraries(ChessApp PRIVATE raylib)

# Copy stockfish.exe to the output directory
add_custom_command(TARGET ChessApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/stockfish.exe"
    "$<TARGET_FILE_DIR:ChessApp>"
)

add_executable(ChessTests tests/test_runner.cpp ${CORE_SOURCES})
add_executable(TestCheck test_check.cpp ${CORE_SOURCES})

# Copy assets to build directory if needed (or just reference them)
# file(COPY src/assets DESTINATION ${CMAKE_BINARY_DIR}/assets)

add_custom_command(TARGET ChessApp 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/../textures"
    "$<TARGET_FILE_DIR:ChessApp>/textures"
)
