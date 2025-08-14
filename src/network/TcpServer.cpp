#include "network/TcpServer.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <memory>

#ifdef SendMessage
#undef SendMessage
#endif

namespace savior_server {
namespace network {

// 类型定义
using ConnectionId = TcpServer::ConnectionId;

// 内部使用的类型定义
using ConnectionHandler = std::function<void(int)>;
using MessageHandler = std::function<void(int, const std::string&)>;
using DisconnectionHandler = std::function<void(int)>;

// TCP连接会话类
class TcpSession : public std::enable_shared_from_this<TcpSession> {
public:
    TcpSession(boost::asio::ip::tcp::socket socket, int id, 
               std::function<void(int, const std::string&)> messageHandler,
               std::function<void(int)> disconnectHandler)
        : m_socket(std::move(socket)), m_connectionId(id), 
          m_messageHandler(messageHandler), m_disconnectHandler(disconnectHandler) {}
    
    void Start() {
        DoRead();
    }
    
    void SendMessage(const std::string& message) {
        auto self(shared_from_this());
        boost::asio::async_write(m_socket,
            boost::asio::buffer(message),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (ec) {
                    std::cerr << "Write error: " << ec.message() << std::endl;
                    HandleDisconnect();
                }
            });
    }
    
    int GetId() const { return m_connectionId; }
    
private:
    void DoRead() {
        auto self(shared_from_this());
        m_socket.async_read_some(boost::asio::buffer(m_data, max_length),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string message(m_data, length);
                    if (m_messageHandler) {
                        m_messageHandler(m_connectionId, message);
                    }
                    DoRead();
                } else {
                    std::cerr << "Read error: " << ec.message() << std::endl;
                    HandleDisconnect();
                }
            });
    }
    
    void HandleDisconnect() {
        if (m_disconnectHandler) {
            m_disconnectHandler(m_connectionId);
        }
    }
    
    boost::asio::ip::tcp::socket m_socket;
    int m_connectionId;
    std::function<void(int, const std::string&)> m_messageHandler;
    std::function<void(int)> m_disconnectHandler;
    enum { max_length = 1024 };
    char m_data[max_length];
};

class TcpServer::Impl {
public:
    Impl() : m_running(false), m_nextConnectionId(1), 
             m_acceptor(m_ioContext), m_socket(m_ioContext) {}
    
    bool Start(uint16_t port, const std::string& address) {
        try {
            std::cout << "Starting TCP server on " << address << ":" << port << std::endl;
            
            auto addr = boost::asio::ip::make_address(address);
            boost::asio::ip::tcp::endpoint endpoint(addr, static_cast<boost::asio::ip::port_type>(port));
            
            m_acceptor.open(endpoint.protocol());
            m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            m_acceptor.bind(endpoint);
            m_acceptor.listen();
            
            m_running = true;
            DoAccept();
            
            std::cout << "TCP server started successfully on " << address << ":" << port << std::endl;
            return true;
        } catch (std::exception& e) {
            std::cerr << "Failed to start TCP server: " << e.what() << std::endl;
            return false;
        }
    }
    
    void Stop() {
        std::cout << "Stopping TCP server..." << std::endl;
        m_running = false;
        
        boost::system::error_code ec;
        m_acceptor.close(ec);
        m_ioContext.stop();
        
        std::lock_guard<std::mutex> lock(m_connectionsMutex);
        m_sessions.clear();
    }
    
    void Run() {
        std::cout << "TCP server is running..." << std::endl;
        try {
            m_ioContext.run();
        } catch (std::exception& e) {
            std::cerr << "TCP server error: " << e.what() << std::endl;
        }
        std::cout << "TCP server stopped." << std::endl;
    }
    
    bool sendMessage(int connectionId, const std::string& message) {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);
        
        auto it = m_sessions.find(connectionId);
        if (it != m_sessions.end()) {
            it->second->SendMessage(message);
            return true;
        }
        
        std::cerr << "Connection " << connectionId << " not found" << std::endl;
        return false;
    }
    
    void disconnectClient(int connectionId) {
        std::lock_guard<std::mutex> lock(m_connectionsMutex);
        
        auto it = m_sessions.find(connectionId);
        if (it != m_sessions.end()) {
            std::cout << "Disconnecting client " << connectionId << std::endl;
            m_sessions.erase(it);
            
            if (m_disconnectionHandler) {
                m_disconnectionHandler(connectionId);
            }
        }
    }
    
    void setConnectionHandler(ConnectionHandler handler) {
        m_connectionHandler = std::move(handler);
    }
    
    void setMessageHandler(MessageHandler handler) {
        m_messageHandler = std::move(handler);
    }
    
    void setDisconnectionHandler(DisconnectionHandler handler) {
        m_disconnectionHandler = std::move(handler);
    }
    
    bool IsRunning() const {
        return m_running;
    }
    
private:
    void DoAccept() {
        m_acceptor.async_accept(m_socket,
            [this](boost::system::error_code ec) {
                if (!ec) {
                    int connectionId = m_nextConnectionId++;
                    
                    auto session = std::make_shared<TcpSession>(
                        std::move(m_socket), connectionId,
                        [this](int id, const std::string& msg) {
                            if (m_messageHandler) {
                                m_messageHandler(id, msg);
                            }
                        },
                        [this](int id) {
                            disconnectClient(id);
                        }
                    );
                    
                    {
                        std::lock_guard<std::mutex> lock(m_connectionsMutex);
                        m_sessions[connectionId] = session;
                    }
                    
                    session->Start();
                    
                    if (m_connectionHandler) {
                        m_connectionHandler(connectionId);
                    }
                    
                    std::cout << "New client connected: " << connectionId << std::endl;
                }
                
                if (m_running) {
                    DoAccept();
                }
            });
    }
    
    std::atomic<bool> m_running;
    int m_nextConnectionId;
    std::mutex m_connectionsMutex;
    
    ConnectionHandler m_connectionHandler;
    MessageHandler m_messageHandler;
    DisconnectionHandler m_disconnectionHandler;
    
    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    std::unordered_map<int, std::shared_ptr<TcpSession>> m_sessions;
};

TcpServer::TcpServer() : pImpl(std::make_unique<Impl>()) {}

TcpServer::~TcpServer() = default;

bool TcpServer::Start(const std::string& host, uint16_t port) {
    return pImpl->Start(port, host);
}

void TcpServer::Stop() {
    pImpl->Stop();
}

void TcpServer::Run() {
    pImpl->Run();
}

bool TcpServer::SendMessage(ConnectionId connectionId, const std::string& message) {
    return pImpl->sendMessage(static_cast<int>(connectionId), message);
}

void TcpServer::Disconnect(ConnectionId connectionId) {
    pImpl->disconnectClient(static_cast<int>(connectionId));
}

void TcpServer::SetOnConnectionCallback(OnConnectionCallback callback) {
    // 需要适配器将int转换为ConnectionId
    auto adapter = [callback](int id) {
        if (callback) callback(static_cast<ConnectionId>(id));
    };
    pImpl->setConnectionHandler(adapter);
}

void TcpServer::SetOnMessageCallback(OnMessageCallback callback) {
    // 需要适配器将int转换为ConnectionId
    auto adapter = [callback](int id, const std::string& msg) {
        if (callback) callback(static_cast<ConnectionId>(id), msg);
    };
    pImpl->setMessageHandler(adapter);
}

void TcpServer::SetOnDisconnectionCallback(OnDisconnectionCallback callback) {
    // 需要适配器将int转换为ConnectionId
    auto adapter = [callback](int id) {
        if (callback) callback(static_cast<ConnectionId>(id));
    };
    pImpl->setDisconnectionHandler(adapter);
}

bool TcpServer::IsRunning() const {
    return pImpl->IsRunning();
}

} // namespace network
} // namespace savior_server