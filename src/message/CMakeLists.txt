# Message模块
set(MESSAGE_SOURCES
    MessageTypes.cpp
)

set(MESSAGE_HEADERS
    ${CMAKE_SOURCE_DIR}/include/message/MessageTypes.hpp
)

add_library(savior_message STATIC
    ${MESSAGE_SOURCES}
    ${MESSAGE_HEADERS}
)

# 设置包含目录
target_include_directories(savior_message
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/boost
)

# 链接依赖
target_link_libraries(savior_message
    PUBLIC
    savior_core
    Boost::system
)