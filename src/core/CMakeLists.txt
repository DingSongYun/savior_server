# Core模块
set(CORE_SOURCES
    Server.cpp
)

set(CORE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/core/Server.hpp
)

add_library(savior_core STATIC
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

# 设置包含目录
target_include_directories(savior_core
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/boost
)

# 链接依赖
target_link_libraries(savior_core
    PUBLIC
    Boost::system
    Boost::thread
)