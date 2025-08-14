#include "TcpClient.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <array>

using boost::asio::ip::tcp;

namespace savior_client {

class TcpClient::Impl {
public:
    Impl() 
        : ioContext_()
        , socket_(ioContext_)
        , connected_(false)
        , running_(false) {
    }

    ~Impl() {
        Stop();
    }

    bool Connect(const std::string& host, int port) {
        try {
            tcp::resolver resolver(ioContext_);
            auto endpoints = resolver.resolve(host, std::to_string(port));
            
            boost::asio::connect(socket_, endpoints);
            connected_ = true;
            
            std::cout << "Connected to server at " << host << ":" << port << std::endl;
            
            // 开始异步读取
            StartRead();
            
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Connection failed: " << e.what() << std::endl;
            if (errorHandler_) {
                errorHandler_("Connection failed: " + std::string(e.what()));
            }
            return false;
        }
    }

    void Disconnect() {
        if (connected_) {
            connected_ = false;
            boost::system::error_code ec;
            socket_.close(ec);
            std::cout << "Disconnected from server" << std::endl;
        }
    }

    bool SendMessage(const std::string& message) {
        if (!connected_) {
            std::cerr << "Not connected to server" << std::endl;
            return false;
        }

        try {
            // 简单的消息格式：长度(4字节) + 消息内容
            uint32_t messageLength = static_cast<uint32_t>(message.length());
            
            std::vector<char> buffer;
            buffer.resize(sizeof(messageLength) + message.length());
            
            // 写入消息长度（网络字节序）
            uint32_t networkLength = htonl(messageLength);
            std::memcpy(buffer.data(), &networkLength, sizeof(networkLength));
            
            // 写入消息内容
            std::memcpy(buffer.data() + sizeof(networkLength), message.c_str(), message.length());
            
            boost::asio::write(socket_, boost::asio::buffer(buffer));
            
            std::cout << "Sent message: " << message << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Send failed: " << e.what() << std::endl;
            if (errorHandler_) {
                errorHandler_("Send failed: " + std::string(e.what()));
            }
            return false;
        }
    }

    bool IsConnected() const {
        return connected_;
    }

    void SetMessageHandler(MessageHandler handler) {
        messageHandler_ = std::move(handler);
    }

    void SetErrorHandler(ErrorHandler handler) {
        errorHandler_ = std::move(handler);
    }

    void Run() {
        if (running_) {
            return;
        }
        
        running_ = true;
        
        try {
            ioContext_.run();
        } catch (const std::exception& e) {
            std::cerr << "Client run error: " << e.what() << std::endl;
            if (errorHandler_) {
                errorHandler_("Client run error: " + std::string(e.what()));
            }
        }
        
        running_ = false;
    }

    void Stop() {
        if (running_) {
            running_ = false;
            Disconnect();
            ioContext_.stop();
        }
    }

private:
    void StartRead() {
        if (!connected_) {
            return;
        }

        // 先读取消息长度
        auto lengthBuffer = std::make_shared<std::array<char, 4>>();
        
        boost::asio::async_read(socket_,
            boost::asio::buffer(*lengthBuffer),
            [this, lengthBuffer](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    // 解析消息长度
                    uint32_t messageLength;
                    std::memcpy(&messageLength, lengthBuffer->data(), sizeof(messageLength));
                    messageLength = ntohl(messageLength);
                    
                    if (messageLength > 0 && messageLength < 65536) { // 限制消息大小
                        ReadMessage(messageLength);
                    } else {
                        std::cerr << "Invalid message length: " << messageLength << std::endl;
                        StartRead(); // 继续读取下一条消息
                    }
                } else {
                    HandleReadError(ec);
                }
            });
    }

    void ReadMessage(uint32_t messageLength) {
        auto messageBuffer = std::make_shared<std::vector<char>>(messageLength);
        
        boost::asio::async_read(socket_,
            boost::asio::buffer(*messageBuffer),
            [this, messageBuffer](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    std::string message(messageBuffer->begin(), messageBuffer->end());
                    std::cout << "Received message: " << message << std::endl;
                    
                    if (messageHandler_) {
                        messageHandler_(message);
                    }
                    
                    StartRead(); // 继续读取下一条消息
                } else {
                    HandleReadError(ec);
                }
            });
    }

    void HandleReadError(const boost::system::error_code& ec) {
        if (ec == boost::asio::error::eof) {
            std::cout << "Server closed connection" << std::endl;
        } else {
            std::cerr << "Read error: " << ec.message() << std::endl;
            if (errorHandler_) {
                errorHandler_("Read error: " + ec.message());
            }
        }
        
        connected_ = false;
    }

private:
    boost::asio::io_context ioContext_;
    tcp::socket socket_;
    bool connected_;
    bool running_;
    MessageHandler messageHandler_;
    ErrorHandler errorHandler_;
};

// TcpClient 实现
TcpClient::TcpClient() : pImpl(std::make_unique<Impl>()) {}

TcpClient::~TcpClient() = default;

bool TcpClient::Connect(const std::string& host, int port) {
    return pImpl->Connect(host, port);
}

void TcpClient::Disconnect() {
    pImpl->Disconnect();
}

bool TcpClient::SendMessage(const std::string& message) {
    return pImpl->SendMessage(message);
}

bool TcpClient::IsConnected() const {
    return pImpl->IsConnected();
}

void TcpClient::SetMessageHandler(MessageHandler handler) {
    pImpl->SetMessageHandler(std::move(handler));
}

void TcpClient::SetErrorHandler(ErrorHandler handler) {
    pImpl->SetErrorHandler(std::move(handler));
}

void TcpClient::Run() {
    pImpl->Run();
}

void TcpClient::Stop() {
    pImpl->Stop();
}

} // namespace savior_client