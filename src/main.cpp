#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include "core/Server.hpp"
#include "network/TcpServer.hpp"
#include "message/MessageTypes.hpp"
#include "entity/Entity.hpp"

int main(int argc, char* argv[])
{
    try
    {
        std::cout << "SaviorServer v1.0.0 Starting..." << std::endl;
        
        // 创建服务器实例
        auto server = std::make_unique<savior_server::core::Server>();
        
        // 初始化服务器
        if (!server->Initialize()) {
            std::cerr << "Failed to initialize server" << std::endl;
            return 1;
        }
        
        // 创建TCP服务器
        auto tcp_server = std::make_unique<savior_server::network::TcpServer>();
        
        // 设置消息处理器
        tcp_server->SetOnConnectionCallback([](savior_server::network::TcpServer::ConnectionId connection_id) {
            std::cout << "Client connected: " << connection_id << std::endl;
        });
        
        tcp_server->SetOnMessageCallback([](savior_server::network::TcpServer::ConnectionId connection_id, const std::string& message) {
            std::cout << "Received message from " << connection_id << ": " << message << std::endl;
        });
        
        tcp_server->SetOnDisconnectionCallback([](savior_server::network::TcpServer::ConnectionId connection_id) {
            std::cout << "Client disconnected: " << connection_id << std::endl;
        });
        
        // 启动TCP服务器
        if (!tcp_server->Start("127.0.0.1", 8080)) {
            std::cerr << "Failed to start TCP server" << std::endl;
            return 1;
        }
        
        // 启动服务器
        if (!server->Start()) {
            std::cerr << "Failed to start server" << std::endl;
            return 1;
        }
        
        std::cout << "Server started successfully on port 8080" << std::endl;
        std::cout << "Press Ctrl+C to stop the server" << std::endl;
        
        // 运行服务器主循环
        server->Run();
        
        // 停止服务器
        tcp_server->Stop();
        server->Stop();
        
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
}