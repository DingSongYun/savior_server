#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <functional>

namespace savior_client {

/**
 * @brief TCP客户端类
 * 
 * 用于连接SaviorServer并进行通信测试
 */
class TcpClient {
public:
    using MessageHandler = std::function<void(const std::string&)>;
    using ErrorHandler = std::function<void(const std::string&)>;

    TcpClient();
    ~TcpClient();

    // 禁用拷贝构造和赋值
    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;

    /**
     * @brief 连接到服务器
     * @param host 服务器地址
     * @param port 服务器端口
     * @return true 连接成功
     * @return false 连接失败
     */
    bool Connect(const std::string& host, int port);

    /**
     * @brief 断开连接
     */
    void Disconnect();

    /**
     * @brief 发送消息
     * @param message 要发送的消息
     * @return true 发送成功
     * @return false 发送失败
     */
    bool SendMessage(const std::string& message);

    /**
     * @brief 检查是否已连接
     * @return true 已连接
     * @return false 未连接
     */
    bool IsConnected() const;

    /**
     * @brief 设置消息处理器
     * @param handler 消息处理函数
     */
    void SetMessageHandler(MessageHandler handler);

    /**
     * @brief 设置错误处理器
     * @param handler 错误处理函数
     */
    void SetErrorHandler(ErrorHandler handler);

    /**
     * @brief 运行客户端（阻塞式）
     */
    void Run();

    /**
     * @brief 停止客户端
     */
    void Stop();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace savior_client