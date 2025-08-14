// Boost.Asio 学习示例 - 第四阶段：高级网络编程
// 编译命令: g++ -std=c++17 04_advanced_networking.cpp -lboost_system -pthread

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// 示例1: 连接池管理
class ConnectionPool {
private:
    asio::io_context& io_;
    std::string host_;
    std::string port_;
    std::queue<std::shared_ptr<tcp::socket>> available_connections_;
    std::mutex mutex_;
    std::size_t max_connections_;
    std::size_t current_connections_;
    
public:
    ConnectionPool(asio::io_context& io, const std::string& host, 
                   const std::string& port, std::size_t max_conn = 10)
        : io_(io), host_(host), port_(port), max_connections_(max_conn), current_connections_(0) {}
    
    void get_connection(std::function<void(std::shared_ptr<tcp::socket>)> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!available_connections_.empty()) {
            auto conn = available_connections_.front();
            available_connections_.pop();
            callback(conn);
            return;
        }
        
        if (current_connections_ < max_connections_) {
            create_new_connection(callback);
        } else {
            // 连接池已满，等待或拒绝
            std::cout << "连接池已满，无法创建新连接" << std::endl;
            callback(nullptr);
        }
    }
    
    void return_connection(std::shared_ptr<tcp::socket> conn) {
        if (conn && conn->is_open()) {
            std::lock_guard<std::mutex> lock(mutex_);
            available_connections_.push(conn);
        } else {
            std::lock_guard<std::mutex> lock(mutex_);
            current_connections_--;
        }
    }
    
private:
    void create_new_connection(std::function<void(std::shared_ptr<tcp::socket>)> callback) {
        auto socket = std::make_shared<tcp::socket>(io_);
        tcp::resolver resolver(io_);
        
        auto endpoints = resolver.resolve(host_, port_);
        asio::async_connect(*socket, endpoints,
            [this, socket, callback](const boost::system::error_code& ec, const tcp::endpoint&) {
                if (!ec) {
                    std::lock_guard<std::mutex> lock(mutex_);
                    current_connections_++;
                    std::cout << "新连接创建成功，当前连接数: " << current_connections_ << std::endl;
                    callback(socket);
                } else {
                    std::cout << "连接创建失败: " << ec.message() << std::endl;
                    callback(nullptr);
                }
            });
    }
};

// 示例2: HTTP 客户端
class HttpClient {
private:
    asio::io_context& io_;
    std::shared_ptr<tcp::socket> socket_;
    asio::streambuf response_;
    
public:
    HttpClient(asio::io_context& io) : io_(io) {}
    
    void get(const std::string& host, const std::string& path, 
             std::function<void(const std::string&)> callback) {
        
        socket_ = std::make_shared<tcp::socket>(io_);
        tcp::resolver resolver(io_);
        
        auto endpoints = resolver.resolve(host, "80");
        asio::async_connect(*socket_, endpoints,
            [this, host, path, callback](const boost::system::error_code& ec, const tcp::endpoint&) {
                if (!ec) {
                    send_request(host, path, callback);
                } else {
                    callback("连接失败: " + ec.message());
                }
            });
    }
    
private:
    void send_request(const std::string& host, const std::string& path,
                     std::function<void(const std::string&)> callback) {
        
        std::ostream request_stream(&response_);
        request_stream << "GET " << path << " HTTP/1.1\r\n";
        request_stream << "Host: " << host << "\r\n";
        request_stream << "Connection: close\r\n\r\n";
        
        asio::async_write(*socket_, response_,
            [this, callback](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    read_response(callback);
                } else {
                    callback("发送请求失败: " + ec.message());
                }
            });
    }
    
    void read_response(std::function<void(const std::string&)> callback) {
        asio::async_read_until(*socket_, response_, "\r\n\r\n",
            [this, callback](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    // 读取响应头
                    std::istream response_stream(&response_);
                    std::string http_version;
                    response_stream >> http_version;
                    unsigned int status_code;
                    response_stream >> status_code;
                    
                    std::string status_message;
                    std::getline(response_stream, status_message);
                    
                    if (status_code == 200) {
                        read_content(callback);
                    } else {
                        callback("HTTP错误: " + std::to_string(status_code) + " " + status_message);
                    }
                } else {
                    callback("读取响应失败: " + ec.message());
                }
            });
    }
    
    void read_content(std::function<void(const std::string&)> callback) {
        asio::async_read(*socket_, response_, asio::transfer_all(),
            [this, callback](const boost::system::error_code& ec, std::size_t) {
                std::string content;
                if (response_.size() > 0) {
                    std::istream response_stream(&response_);
                    content.assign(std::istreambuf_iterator<char>(response_stream),
                                  std::istreambuf_iterator<char>());
                }
                callback(content);
            });
    }
};

// 示例3: 多线程服务器
class MultiThreadServer {
private:
    asio::io_context& io_;
    tcp::acceptor acceptor_;
    std::vector<std::thread> thread_pool_;
    std::atomic<bool> running_;
    
public:
    MultiThreadServer(asio::io_context& io, short port, std::size_t thread_count = 4)
        : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port)), running_(true) {
        
        // 创建线程池
        for (std::size_t i = 0; i < thread_count; ++i) {
            thread_pool_.emplace_back([this]() {
                while (running_) {
                    try {
                        io_.run();
                    } catch (const std::exception& e) {
                        std::cerr << "线程异常: " << e.what() << std::endl;
                    }
                }
            });
        }
    }
    
    ~MultiThreadServer() {
        stop();
    }
    
    void start() {
        std::cout << "多线程服务器启动，端口: " << acceptor_.local_endpoint().port() 
                  << "，线程数: " << thread_pool_.size() << std::endl;
        start_accept();
    }
    
    void stop() {
        running_ = false;
        io_.stop();
        
        for (auto& thread : thread_pool_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    
private:
    void start_accept() {
        acceptor_.async_accept(
            [this](const boost::system::error_code& ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "新连接来自: " << socket.remote_endpoint() 
                              << " [线程ID: " << std::this_thread::get_id() << "]" << std::endl;
                    
                    // 处理连接（简单的echo服务）
                    handle_client(std::move(socket));
                }
                
                if (running_) {
                    start_accept();
                }
            });
    }
    
    void handle_client(tcp::socket socket) {
        auto buffer = std::make_shared<std::array<char, 1024>>();
        
        socket.async_read_some(asio::buffer(*buffer),
            [this, socket = std::move(socket), buffer](const boost::system::error_code& ec, std::size_t length) mutable {
                if (!ec) {
                    std::cout << "收到数据: " << std::string(buffer->data(), length)
                              << " [线程ID: " << std::this_thread::get_id() << "]" << std::endl;
                    
                    // Echo 回客户端
                    asio::async_write(socket, asio::buffer(*buffer, length),
                        [socket = std::move(socket)](const boost::system::error_code&, std::size_t) {
                            // 连接处理完成
                        });
                }
            });
    }
};

// 示例4: 带超时的操作
class TimeoutOperation {
private:
    asio::io_context& io_;
    asio::steady_timer timer_;
    tcp::socket socket_;
    bool completed_;
    
public:
    TimeoutOperation(asio::io_context& io) 
        : io_(io), timer_(io), socket_(io), completed_(false) {}
    
    void connect_with_timeout(const std::string& host, const std::string& port,
                             std::chrono::seconds timeout,
                             std::function<void(bool, const std::string&)> callback) {
        
        tcp::resolver resolver(io_);
        auto endpoints = resolver.resolve(host, port);
        
        // 设置超时定时器
        timer_.expires_after(timeout);
        timer_.async_wait([this, callback](const boost::system::error_code& ec) {
            if (!ec && !completed_) {
                completed_ = true;
                socket_.close();
                callback(false, "连接超时");
            }
        });
        
        // 尝试连接
        asio::async_connect(socket_, endpoints,
            [this, callback](const boost::system::error_code& ec, const tcp::endpoint&) {
                if (!completed_) {
                    completed_ = true;
                    timer_.cancel();
                    
                    if (!ec) {
                        callback(true, "连接成功");
                    } else {
                        callback(false, "连接失败: " + ec.message());
                    }
                }
            });
    }
};

// 示例5: 简单的负载均衡器
class LoadBalancer {
private:
    asio::io_context& io_;
    std::vector<std::pair<std::string, std::string>> servers_;
    std::size_t current_server_;
    
public:
    LoadBalancer(asio::io_context& io) : io_(io), current_server_(0) {}
    
    void add_server(const std::string& host, const std::string& port) {
        servers_.emplace_back(host, port);
        std::cout << "添加服务器: " << host << ":" << port << std::endl;
    }
    
    void forward_request(const std::string& data,
                        std::function<void(const std::string&)> callback) {
        if (servers_.empty()) {
            callback("没有可用的服务器");
            return;
        }
        
        // 轮询选择服务器
        auto& server = servers_[current_server_];
        current_server_ = (current_server_ + 1) % servers_.size();
        
        std::cout << "转发请求到: " << server.first << ":" << server.second << std::endl;
        
        auto socket = std::make_shared<tcp::socket>(io_);
        tcp::resolver resolver(io_);
        
        auto endpoints = resolver.resolve(server.first, server.second);
        asio::async_connect(*socket, endpoints,
            [this, socket, data, callback](const boost::system::error_code& ec, const tcp::endpoint&) {
                if (!ec) {
                    send_and_receive(socket, data, callback);
                } else {
                    callback("转发失败: " + ec.message());
                }
            });
    }
    
private:
    void send_and_receive(std::shared_ptr<tcp::socket> socket, const std::string& data,
                         std::function<void(const std::string&)> callback) {
        
        asio::async_write(*socket, asio::buffer(data),
            [this, socket, callback](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    auto buffer = std::make_shared<std::array<char, 1024>>();
                    socket->async_read_some(asio::buffer(*buffer),
                        [buffer, callback](const boost::system::error_code& ec, std::size_t length) {
                            if (!ec) {
                                callback(std::string(buffer->data(), length));
                            } else {
                                callback("读取响应失败: " + ec.message());
                            }
                        });
                } else {
                    callback("发送数据失败: " + ec.message());
                }
            });
    }
};

// 测试函数
void test_connection_pool() {
    std::cout << "\n=== 连接池测试 ===" << std::endl;
    
    asio::io_context io;
    ConnectionPool pool(io, "www.google.com", "80", 3);
    
    for (int i = 0; i < 5; ++i) {
        pool.get_connection([&pool, i](std::shared_ptr<tcp::socket> conn) {
            if (conn) {
                std::cout << "获取连接 " << i << " 成功" << std::endl;
                // 模拟使用连接
                asio::steady_timer timer(conn->get_executor(), std::chrono::seconds(1));
                timer.async_wait([&pool, conn](const boost::system::error_code&) {
                    pool.return_connection(conn);
                    std::cout << "归还连接" << std::endl;
                });
            } else {
                std::cout << "获取连接 " << i << " 失败" << std::endl;
            }
        });
    }
    
    io.run();
}

void test_http_client() {
    std::cout << "\n=== HTTP 客户端测试 ===" << std::endl;
    
    asio::io_context io;
    HttpClient client(io);
    
    client.get("httpbin.org", "/get", [](const std::string& response) {
        std::cout << "HTTP 响应:\n" << response.substr(0, 500) << "..." << std::endl;
    });
    
    io.run();
}

void test_timeout_operation() {
    std::cout << "\n=== 超时操作测试 ===" << std::endl;
    
    asio::io_context io;
    TimeoutOperation timeout_op(io);
    
    timeout_op.connect_with_timeout("www.google.com", "80", std::chrono::seconds(5),
        [](bool success, const std::string& message) {
            std::cout << "连接结果: " << (success ? "成功" : "失败") 
                      << " - " << message << std::endl;
        });
    
    io.run();
}

int main() {
    std::cout << "Boost.Asio 高级网络编程示例" << std::endl;
    std::cout << "==============================" << std::endl;
    
    try {
        std::cout << "\n选择测试模式:" << std::endl;
        std::cout << "1. 连接池测试" << std::endl;
        std::cout << "2. HTTP 客户端测试" << std::endl;
        std::cout << "3. 多线程服务器测试" << std::endl;
        std::cout << "4. 超时操作测试" << std::endl;
        std::cout << "请输入选择 (1-4): ";
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                test_connection_pool();
                break;
            case 2:
                test_http_client();
                break;
            case 3: {
                asio::io_context io;
                MultiThreadServer server(io, 8080, 4);
                server.start();
                
                // 运行10秒后停止
                asio::steady_timer timer(io, std::chrono::seconds(10));
                timer.async_wait([&server](const boost::system::error_code&) {
                    server.stop();
                });
                
                io.run();
                break;
            }
            case 4:
                test_timeout_operation();
                break;
            default:
                std::cout << "无效选择" << std::endl;
                break;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "程序异常: " << e.what() << std::endl;
    }
    
    std::cout << "\n高级网络编程示例运行完成！" << std::endl;
    return 0;
}