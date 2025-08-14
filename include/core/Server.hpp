#pragma once

#include <memory>
#include <string>

namespace savior_server {
namespace core {

/**
 * @brief 服务器核心类
 * 
 * 负责服务器的初始化、启动、停止和运行
 * 使用PIMPL模式隐藏实现细节
 */
class Server {
public:
    Server();
    ~Server();

    // 禁用拷贝构造和赋值
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    /**
     * @brief 初始化服务器
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool Initialize();

    /**
     * @brief 启动服务器
     * @return true 启动成功
     * @return false 启动失败
     */
    bool Start();

    /**
     * @brief 停止服务器
     */
    void Stop();

    /**
     * @brief 运行服务器主循环
     */
    void Run();

    /**
     * @brief 检查服务器是否正在运行
     * @return true 正在运行
     * @return false 未运行
     */
    bool IsRunning() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace core
} // namespace savior_server