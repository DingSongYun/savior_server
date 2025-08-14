cmake_minimum_required(VERSION 3.20)
project(SaviorServer VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX- /utf-8)
    add_compile_definitions(_WIN32_WINNT=0x0601)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置Boost路径
set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/libs/boost)
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# 查找依赖库
find_package(Boost COMPONENTS system filesystem thread)
find_package(Threads REQUIRED)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/boost
)

# 如果找不到Boost，使用头文件模式
if(NOT Boost_FOUND)
    message(STATUS "Using Boost header-only mode")
    set(Boost_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libs/boost)
    add_library(Boost::headers INTERFACE IMPORTED)
    target_include_directories(Boost::headers INTERFACE ${Boost_INCLUDE_DIRS})
    
    # 创建虚拟目标
    add_library(Boost::system INTERFACE IMPORTED)
    add_library(Boost::filesystem INTERFACE IMPORTED)
    add_library(Boost::thread INTERFACE IMPORTED)
endif()

# 添加子目录
add_subdirectory(src/core)
add_subdirectory(src/network)
add_subdirectory(src/message)
add_subdirectory(src/entity)

# 主可执行文件
add_executable(savior_server
    src/main.cpp
)

# 链接库
target_link_libraries(savior_server
    PRIVATE
    savior_core
    savior_network
    savior_message
    savior_entity
    Boost::system
    Boost::filesystem
    Boost::thread
    Threads::Threads
)

# 客户端Demo
add_subdirectory(client_demo)

# 测试
enable_testing()
add_subdirectory(tests)