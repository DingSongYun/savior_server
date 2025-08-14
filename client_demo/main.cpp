#include "TcpClient.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

using namespace savior_client;

int main() {
    std::cout << "=== SaviorServer Client Demo ===" << std::endl;
    
    TcpClient client;
    
    // 设置消息处理器
    client.SetMessageHandler([](const std::string& message) {
        std::cout << "[SERVER] " << message << std::endl;
    });
    
    // 设置错误处理器
    client.SetErrorHandler([](const std::string& error) {
        std::cerr << "[ERROR] " << error << std::endl;
    });
    
    // 连接到服务器
    std::string host = "127.0.0.1";
    int port = 8080;
    
    std::cout << "Connecting to " << host << ":" << port << "..." << std::endl;
    
    if (!client.Connect(host, port)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    // 在单独的线程中运行客户端
    std::thread clientThread([&client]() {
        client.Run();
    });
    
    // 发送测试消息
    std::cout << "\nSending test messages..." << std::endl;
    
    // 发送几条测试消息
    client.SendMessage("Hello, SaviorServer!");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    client.SendMessage("This is a test message from client demo");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    client.SendMessage("Testing connection...");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 交互式模式
    std::cout << "\nEntering interactive mode. Type 'quit' to exit." << std::endl;
    std::cout << "Enter messages to send to server:" << std::endl;
    
    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "quit" || input == "exit") {
            break;
        }
        
        if (input == "status") {
            std::cout << "Connection status: " << (client.IsConnected() ? "Connected" : "Disconnected") << std::endl;
            continue;
        }
        
        if (input == "disconnect") {
            client.Disconnect();
            std::cout << "Disconnected from server" << std::endl;
            continue;
        }
        
        if (input == "reconnect") {
            if (!client.IsConnected()) {
                if (client.Connect(host, port)) {
                    std::cout << "Reconnected to server" << std::endl;
                } else {
                    std::cout << "Failed to reconnect" << std::endl;
                }
            } else {
                std::cout << "Already connected" << std::endl;
            }
            continue;
        }
        
        if (!input.empty()) {
            if (client.IsConnected()) {
                client.SendMessage(input);
            } else {
                std::cout << "Not connected to server. Type 'reconnect' to reconnect." << std::endl;
            }
        }
    }
    
    // 清理
    std::cout << "\nShutting down client..." << std::endl;
    client.Stop();
    
    if (clientThread.joinable()) {
        clientThread.join();
    }
    
    std::cout << "Client demo finished." << std::endl;
    return 0;
}