// Boost.Asio 学习示例 - 第三阶段：网络编程基础
// 编译命令: g++ -std=c++17 03_networking_basics.cpp -lboost_system -pthread

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <array>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;
using udp = asio::ip::udp;

// 示例1: TCP 客户端
class TcpClient {
private:
    asio::io_context& io_;
    tcp::socket socket_;
    std::array<char, 1024> buffer_;
    
public:
    TcpClient(asio::io_context& io) : io_(io), socket_(io) {}
    
    void connect(const std::string& host, const std::string& port) {
        tcp::resolver resolver(io_);
        auto endpoints = resolver.resolve(host, port);
        
        asio::async_connect(socket_, endpoints,
            [this](const boost::system::error_code& ec, const tcp::endpoint&) {
                if (!ec) {
                    std::cout << "TCP客户端连接成功" << std::endl;
                    start_read();
                } else {
                    std::cout << "TCP客户端连接失败: " << ec.message() << std::endl;
                }
            });
    }
    
    void send_message(const std::string& message) {
        auto msg = std::make_shared<std::string>(message + "\n");
        asio::async_write(socket_, asio::buffer(*msg),
            [this, msg](const boost::system::error_code& ec, std::size_t) {
                if (ec) {
                    std::cout << "发送消息失败: " << ec.message() << std::endl;
                }
            });
    }
    
private:
    void start_read() {
        socket_.async_read_some(asio::buffer(buffer_),
            [this](const boost::system::error_code& ec, std::size_t length) {
                if (!ec) {
                    std::cout << "收到服务器消息: " 
                              << std::string(buffer_.data(), length);
                    start_read();  // 继续读取
                } else {
                    std::cout << "读取失败: " << ec.message() << std::endl;
                }
            });
    }
};

// 示例2: TCP 服务器
class TcpSession : public std::enable_shared_from_this<TcpSession> {
private:
    tcp::socket socket_;
    std::array<char, 1024> buffer_;
    
public:
    TcpSession(tcp::socket socket) : socket_(std::move(socket)) {}
    
    void start() {
        std::cout << "新的TCP连接: " << socket_.remote_endpoint() << std::endl;
        start_read();
    }
    
private:
    void start_read() {
        auto self = shared_from_this();
        socket_.async_read_some(asio::buffer(buffer_),
            [this, self](const boost::system::error_code& ec, std::size_t length) {
                if (!ec) {
                    std::cout << "收到客户端消息: " 
                              << std::string(buffer_.data(), length);
                    
                    // Echo 回客户端
                    start_write(length);
                } else {
                    std::cout << "客户端断开连接: " << ec.message() << std::endl;
                }
            });
    }
    
    void start_write(std::size_t length) {
        auto self = shared_from_this();
        asio::async_write(socket_, asio::buffer(buffer_, length),
            [this, self](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    start_read();  // 继续读取
                } else {
                    std::cout << "写入失败: " << ec.message() << std::endl;
                }
            });
    }
};

class TcpServer {
private:
    asio::io_context& io_;
    tcp::acceptor acceptor_;
    
public:
    TcpServer(asio::io_context& io, short port) 
        : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port)) {}
    
    void start() {
        std::cout << "TCP服务器启动，监听端口: " 
                  << acceptor_.local_endpoint().port() << std::endl;
        start_accept();
    }
    
private:
    void start_accept() {
        acceptor_.async_accept(
            [this](const boost::system::error_code& ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<TcpSession>(std::move(socket))->start();
                }
                start_accept();  // 继续接受新连接
            });
    }
};

// 示例3: UDP 客户端
class UdpClient {
private:
    asio::io_context& io_;
    udp::socket socket_;
    udp::endpoint server_endpoint_;
    std::array<char, 1024> buffer_;
    
public:
    UdpClient(asio::io_context& io) : io_(io), socket_(io, udp::endpoint(udp::v4(), 0)) {}
    
    void connect(const std::string& host, const std::string& port) {
        udp::resolver resolver(io_);
        server_endpoint_ = *resolver.resolve(udp::v4(), host, port).begin();
        
        std::cout << "UDP客户端准备就绪，服务器: " << server_endpoint_ << std::endl;
        start_receive();
    }
    
    void send_message(const std::string& message) {
        socket_.async_send_to(asio::buffer(message), server_endpoint_,
            [](const boost::system::error_code& ec, std::size_t) {
                if (ec) {
                    std::cout << "UDP发送失败: " << ec.message() << std::endl;
                }
            });
    }
    
private:
    void start_receive() {
        socket_.async_receive_from(asio::buffer(buffer_), server_endpoint_,
            [this](const boost::system::error_code& ec, std::size_t length) {
                if (!ec) {
                    std::cout << "收到UDP消息: " 
                              << std::string(buffer_.data(), length) << std::endl;
                    start_receive();  // 继续接收
                } else {
                    std::cout << "UDP接收失败: " << ec.message() << std::endl;
                }
            });
    }
};

// 示例4: UDP 服务器
class UdpServer {
private:
    asio::io_context& io_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::array<char, 1024> buffer_;
    
public:
    UdpServer(asio::io_context& io, short port) 
        : io_(io), socket_(io, udp::endpoint(udp::v4(), port)) {}
    
    void start() {
        std::cout << "UDP服务器启动，监听端口: " 
                  << socket_.local_endpoint().port() << std::endl;
        start_receive();
    }
    
private:
    void start_receive() {
        socket_.async_receive_from(asio::buffer(buffer_), remote_endpoint_,
            [this](const boost::system::error_code& ec, std::size_t length) {
                if (!ec) {
                    std::cout << "收到UDP消息来自 " << remote_endpoint_ 
                              << ": " << std::string(buffer_.data(), length) << std::endl;
                    
                    // Echo 回客户端
                    start_send(length);
                } else {
                    std::cout << "UDP接收失败: " << ec.message() << std::endl;
                }
            });
    }
    
    void start_send(std::size_t length) {
        socket_.async_send_to(asio::buffer(buffer_, length), remote_endpoint_,
            [this](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    start_receive();  // 继续接收
                } else {
                    std::cout << "UDP发送失败: " << ec.message() << std::endl;
                }
            });
    }
};

// 示例5: 域名解析
void example_dns_resolution() {
    std::cout << "\n=== 域名解析示例 ===" << std::endl;
    
    asio::io_context io;
    tcp::resolver resolver(io);
    
    // 异步解析域名
    resolver.async_resolve("www.google.com", "80",
        [](const boost::system::error_code& ec, tcp::resolver::results_type results) {
            if (!ec) {
                std::cout << "域名解析成功:" << std::endl;
                for (const auto& endpoint : results) {
                    std::cout << "  " << endpoint.endpoint() << std::endl;
                }
            } else {
                std::cout << "域名解析失败: " << ec.message() << std::endl;
            }
        });
    
    io.run();
}

// 测试函数
void test_tcp_echo_server() {
    std::cout << "\n=== TCP Echo 服务器测试 ===" << std::endl;
    
    asio::io_context io;
    TcpServer server(io, 8080);
    server.start();
    
    // 运行一段时间后停止
    asio::steady_timer timer(io, std::chrono::seconds(10));
    timer.async_wait([&io](const boost::system::error_code&) {
        std::cout << "停止服务器" << std::endl;
        io.stop();
    });
    
    io.run();
}

void test_udp_echo_server() {
    std::cout << "\n=== UDP Echo 服务器测试 ===" << std::endl;
    
    asio::io_context io;
    UdpServer server(io, 8081);
    server.start();
    
    // 运行一段时间后停止
    asio::steady_timer timer(io, std::chrono::seconds(10));
    timer.async_wait([&io](const boost::system::error_code&) {
        std::cout << "停止服务器" << std::endl;
        io.stop();
    });
    
    io.run();
}

int main() {
    std::cout << "Boost.Asio 网络编程基础示例" << std::endl;
    std::cout << "=============================" << std::endl;
    
    try {
        example_dns_resolution();
        
        std::cout << "\n选择测试模式:" << std::endl;
        std::cout << "1. TCP Echo 服务器" << std::endl;
        std::cout << "2. UDP Echo 服务器" << std::endl;
        std::cout << "请输入选择 (1 或 2): ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                test_tcp_echo_server();
                break;
            case 2:
                test_udp_echo_server();
                break;
            default:
                std::cout << "无效选择，运行 TCP 服务器" << std::endl;
                test_tcp_echo_server();
                break;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
    }
    
    std::cout << "\n网络编程示例运行完成！" << std::endl;
    return 0;
}