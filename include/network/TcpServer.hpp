#pragma once

#include <memory>
#include <string>
#include <functional>
#include <cstdint>

namespace savior_server {
namespace network {

/**
 * @brief TCP服务器类
 * 
 * 基于Boost.Asio的异步TCP服务器
 * 提供连接管理、消息收发等功能
 */
class TcpServer {
public:
    // 连接ID类型
    using ConnectionId = uint64_t;
    
    // 回调函数类型定义
    using OnConnectionCallback = std::function<void(ConnectionId)>;
    using OnMessageCallback = std::function<void(ConnectionId, const std::string&)>;
    using OnDisconnectionCallback = std::function<void(ConnectionId)>;

    TcpServer();
    ~TcpServer();

    // 禁用拷贝构造和赋值
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    /**
     * @brief 启动TCP服务器
     * @param host 监听地址
     * @param port 监听端口
     * @return true 启动成功
     * @return false 启动失败
     */
    bool Start(const std::string& host, uint16_t port);

    /**
     * @brief 停止TCP服务器
     */
    void Stop();

    /**
     * @brief 运行服务器事件循环
     */
    void Run();

    /**
     * @brief 发送消息到指定连接
     * @param connectionId 连接ID
     * @param message 消息内容
     * @return true 发送成功
     * @return false 发送失败
     */
    bool SendMessage(ConnectionId connectionId, const std::string& message);

    /**
     * @brief 断开指定连接
     * @param connectionId 连接ID
     */
    void Disconnect(ConnectionId connectionId);

    /**
     * @brief 设置连接回调函数
     * @param callback 回调函数
     */
    void SetOnConnectionCallback(OnConnectionCallback callback);

    /**
     * @brief 设置消息回调函数
     * @param callback 回调函数
     */
    void SetOnMessageCallback(OnMessageCallback callback);

    /**
     * @brief 设置断开连接回调函数
     * @param callback 回调函数
     */
    void SetOnDisconnectionCallback(OnDisconnectionCallback callback);

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

} // namespace network
} // namespace savior_server