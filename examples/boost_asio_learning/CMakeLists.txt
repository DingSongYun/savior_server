cmake_minimum_required(VERSION 3.16)
project(BoostAsioLearning)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build_output/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build_output/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build_output/lib)

# 优先使用本地 Boost 源码
set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/../../libs/boost")
set(Boost_NO_SYSTEM_PATHS ON)

# 查找 Boost 库 (header-only 模式)
find_package(Boost REQUIRED)

if(Boost_FOUND)
    message(STATUS "Boost found!")
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    # 对于 header-only 库，不需要链接库
    set(Boost_LIBRARIES "")
else()
    message(FATAL_ERROR "Boost not found!")
endif()

# 包含 Boost 头文件
include_directories(${Boost_INCLUDE_DIRS})

# 使用 basic_examples 目录中的真正示例文件
add_executable(01_basic_concepts basic_examples/01_basic_concepts.cpp)
add_executable(02_timers basic_examples/02_timers.cpp)
add_executable(03_networking_basics basic_examples/03_networking_basics.cpp)
add_executable(04_advanced_networking basic_examples/04_advanced_networking.cpp)
add_executable(05_strand_thread_safety basic_examples/05_strand_thread_safety.cpp)
add_executable(io_context_analysis basic_examples/io_context_analysis.cpp)
add_executable(io_concept_analysis basic_examples/io_concept_analysis.cpp)
add_executable(io_multiplexing_analysis basic_examples/io_multiplexing_analysis.cpp)

# 链接 Boost 库
target_link_libraries(01_basic_concepts ${Boost_LIBRARIES})
target_link_libraries(02_timers ${Boost_LIBRARIES})
target_link_libraries(03_networking_basics ${Boost_LIBRARIES})
target_link_libraries(04_advanced_networking ${Boost_LIBRARIES})
target_link_libraries(05_strand_thread_safety ${Boost_LIBRARIES})
target_link_libraries(io_context_analysis ${Boost_LIBRARIES})
target_link_libraries(io_concept_analysis ${Boost_LIBRARIES})
target_link_libraries(io_multiplexing_analysis ${Boost_LIBRARIES})

# Windows 特定设置
if(WIN32)
    target_link_libraries(03_networking_basics ws2_32)
    target_link_libraries(04_advanced_networking ws2_32)
    target_link_libraries(05_strand_thread_safety ws2_32)
endif()

# 为了向后兼容，保留一个简单的测试程序
file(WRITE "${CMAKE_BINARY_DIR}/boost_test.cpp" "
#include <boost/asio.hpp>
#include <iostream>
#include <boost/version.hpp>

int main() {
    std::cout << \"Boost version: \" << BOOST_VERSION / 100000 << \".\"
              << BOOST_VERSION / 100 % 1000 << \".\"
              << BOOST_VERSION % 100 << std::endl;
    
    boost::asio::io_context io;
    boost::asio::steady_timer timer(io, boost::asio::chrono::seconds(1));
    
    std::cout << \"Created io_context and steady_timer successfully!\" << std::endl;
    std::cout << \"\\nTo run the learning examples, use:\" << std::endl;
    std::cout << \"  01_basic_concepts.exe    - 基础概念\" << std::endl;
    std::cout << \"  02_timers.exe           - 定时器\" << std::endl;
    std::cout << \"  03_networking_basics.exe - 网络基础\" << std::endl;
    std::cout << \"  04_advanced_networking.exe - 高级网络\" << std::endl;
    std::cout << \"  05_strand_thread_safety.exe - 线程安全\" << std::endl;
    
    return 0;
}
")

add_executable(boost_test ${CMAKE_BINARY_DIR}/boost_test.cpp)
target_link_libraries(boost_test ${Boost_LIBRARIES})