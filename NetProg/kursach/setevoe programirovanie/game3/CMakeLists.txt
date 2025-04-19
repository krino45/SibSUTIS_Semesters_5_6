cmake_minimum_required(VERSION 3.10)
project(pong)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Common sources
set(COMMON_SOURCES
    common/game_state.h
    common/network.h
    common/network.cpp
    common/utils.h
)

# Common headers
include_directories(
    common
    client
    server
)

# Client executable
add_executable(pong_client
    client/main.cpp
    client/game.cpp
    client/render.cpp
    client/input.cpp
    client/network.cpp
    ${COMMON_SOURCES}
)

# Server executable
add_executable(pong_server
    server/main.cpp
    server/matchmaker.cpp
    server/network.cpp
    server/game.cpp
    ${COMMON_SOURCES}
)

# Platform-specific settings
if(UNIX)
    target_link_libraries(pong_client PRIVATE pthread)
    target_link_libraries(pong_server PRIVATE pthread)
endif()
