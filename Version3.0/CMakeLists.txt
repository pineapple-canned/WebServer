cmake_minimum_required(VERSION 3.10)
project(ThreadTcpServer)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SRC_LIST
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/ThreadTcpServer.cpp
)

add_executable(server ${SRC_LIST})