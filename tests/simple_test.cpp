#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include "../include/core/Server.hpp"

using namespace savior_server::core;

int main()
{
    std::cout << "Running server startup tests..." << std::endl;
    
    try {
        // 创建服务器实例
        Server server;
        
        // 测试服务器初始化
        std::cout << "Testing server initialization..." << std::endl;
        bool initResult = server.Initialize();
        assert(initResult == true);
        std::cout << "Server initialization: PASSED" << std::endl;
        
        // 测试服务器启动
        std::cout << "Testing server startup..." << std::endl;
        bool startResult = server.Start();
        assert(startResult == true);
        std::cout << "Server startup: PASSED" << std::endl;
        
        // 验证服务器运行状态
        std::cout << "Testing server running status..." << std::endl;
        bool isRunning = server.IsRunning();
        assert(isRunning == true);
        std::cout << "Server running status: PASSED" << std::endl;
        
        // 让服务器运行一小段时间
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // 测试服务器停止
        std::cout << "Testing server shutdown..." << std::endl;
        server.Stop();
        
        // 验证服务器已停止
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        bool isStoppedAfterStop = !server.IsRunning();
        assert(isStoppedAfterStop == true);
        std::cout << "Server shutdown: PASSED" << std::endl;
        
        std::cout << "All server tests passed!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
    
    return 0;
}