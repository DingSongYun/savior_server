# 客户端Demo CMakeLists.txt
cmake_minimum_required(VERSION 3.20)

project(SaviorClientDemo)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 使用主项目已配置的Boost设置
# Boost目标已在主CMakeLists.txt中定义

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/libs/boost
)

# 创建客户端可执行文件
add_executable(client_demo
    main.cpp
    TcpClient.cpp
)

# 链接库
target_link_libraries(client_demo
    PRIVATE
    ${Boost_LIBRARIES}
    savior_message  # 使用服务器的消息系统
)

# 设置输出目录
set_target_properties(client_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Windows特定设置
if(WIN32)
    target_link_libraries(client_demo PRIVATE ws2_32 wsock32)
endif()