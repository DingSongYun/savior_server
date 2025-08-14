# Network模块
set(NETWORK_SOURCES
    TcpServer.cpp
)

set(NETWORK_HEADERS
    ${CMAKE_SOURCE_DIR}/include/network/TcpServer.hpp
)

add_library(savior_network STATIC
    ${NETWORK_SOURCES}
    ${NETWORK_HEADERS}
)

# 设置包含目录
target_include_directories(savior_network
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/boost
)

# 链接依赖
target_link_libraries(savior_network
    PUBLIC
    savior_core
    Boost::system
    Boost::thread
    Threads::Threads
)