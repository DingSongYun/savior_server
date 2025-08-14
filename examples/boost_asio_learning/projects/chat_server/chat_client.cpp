// 聊天客户端实现
// 编译命令: g++ -std=c++17 chat_client.cpp -lboost_system -pthread -o chat_client

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <deque>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// 消息类型定义（与服务器保持一致）
struct ChatMessage {
    enum Type {
        JOIN,           // 用户加入
        LEAVE,          // 用户离开
        MESSAGE,        // 普通消息
        PRIVATE_MSG,    // 私聊消息
        ROOM_JOIN,      // 加入房间
        ROOM_LEAVE,     // 离开房间
        USER_LIST,      // 用户列表
        ROOM_LIST       // 房间列表
    };
    
    Type type;
    std::string sender;
    std::string receiver;
    std::string content;
    std::string timestamp;
    
    std::string serialize() const {
        return std::to_string(static_cast<int>(type)) + "|" + 
               sender + "|" + receiver + "|" + content + "|" + timestamp + "\n";
    }
    
    static ChatMessage deserialize(const std::string& data) {
        ChatMessage msg;
        std::vector<std::string> parts;
        std::string current;
        
        for (char c : data) {
            if (c == '|') {
                parts.push_back(current);
                current.clear();
            } else if (c != '\n') {
                current += c;
            }
        }
        if (!current.empty()) {
            parts.push_back(current);
        }
        
        if (parts.size() >= 5) {
            msg.type = static_cast<Type>(std::stoi(parts[0]));
            msg.sender = parts[1];
            msg.receiver = parts[2];
            msg.content = parts[3];
            msg.timestamp = parts[4];
        }
        
        return msg;
    }
};

class ChatClient {
private:
    asio::io_context& io_;
    tcp::socket socket_;
    std::string username_;
    std::string current_room_;
    std::array<char, 1024> read_buffer_;
    std::deque<std::string> write_queue_;
    asio::io_context::strand strand_;
    bool connected_;
    
public:
    ChatClient(asio::io_context& io) 
        : io_(io), socket_(io), strand_(io), connected_(false) {}
    
    void connect(const std::string& host, const std::string& port, const std::string& username) {
        username_ = username;
        
        tcp::resolver resolver(io_);
        auto endpoints = resolver.resolve(host, port);
        
        asio::async_connect(socket_, endpoints,
            [this](const boost::system::error_code& ec, const tcp::endpoint&) {
                if (!ec) {
                    connected_ = true;
                    std::cout << "连接到服务器成功！" << std::endl;
                    
                    // 发送登录消息
                    ChatMessage join_msg;
                    join_msg.type = ChatMessage::JOIN;
                    join_msg.sender = username_;
                    send_message(join_msg);
                    
                    start_read();
                    show_help();
                } else {
                    std::cout << "连接失败: " << ec.message() << std::endl;
                }
            });
    }
    
    void send_message(const ChatMessage& msg) {
        strand_.post([this, msg]() {
            bool write_in_progress = !write_queue_.empty();
            write_queue_.push_back(msg.serialize());
            if (!write_in_progress) {
                start_write();
            }
        });
    }
    
    void send_text_message(const std::string& text) {
        if (!connected_) {
            std::cout << "未连接到服务器" << std::endl;
            return;
        }
        
        if (text.empty()) return;
        
        // 解析命令
        if (text[0] == '/') {
            handle_command(text);
        } else {
            // 普通消息
            ChatMessage msg;
            msg.type = ChatMessage::MESSAGE;
            msg.sender = username_;
            msg.content = text;
            send_message(msg);
        }
    }
    
    void disconnect() {
        if (connected_) {
            connected_ = false;
            socket_.close();
            std::cout << "已断开连接" << std::endl;
        }
    }
    
    bool is_connected() const { return connected_; }
    
private:
    void start_read() {
        socket_.async_read_some(asio::buffer(read_buffer_),
            strand_.wrap([this](const boost::system::error_code& ec, std::size_t length) {
                if (!ec) {
                    std::string data(read_buffer_.data(), length);
                    handle_received_message(data);
                    start_read();
                } else {
                    std::cout << "连接断开: " << ec.message() << std::endl;
                    connected_ = false;
                }
            }));
    }
    
    void start_write() {
        if (write_queue_.empty()) return;
        
        asio::async_write(socket_, asio::buffer(write_queue_.front()),
            strand_.wrap([this](const boost::system::error_code& ec, std::size_t) {
                if (!ec) {
                    write_queue_.pop_front();
                    if (!write_queue_.empty()) {
                        start_write();
                    }
                } else {
                    std::cout << "发送失败: " << ec.message() << std::endl;
                    connected_ = false;
                }
            }));
    }
    
    void handle_received_message(const std::string& data) {
        auto msg = ChatMessage::deserialize(data);
        
        switch (msg.type) {
            case ChatMessage::JOIN:
                if (msg.sender != username_) {
                    std::cout << "[系统] " << msg.content << std::endl;
                }
                break;
                
            case ChatMessage::LEAVE:
                std::cout << "[系统] " << msg.content << std::endl;
                break;
                
            case ChatMessage::MESSAGE:
                if (msg.sender.empty()) {
                    // 系统消息
                    std::cout << "[系统] " << msg.content << std::endl;
                } else if (msg.sender != username_) {
                    // 其他用户消息
                    std::cout << "[" << msg.timestamp << "] " << msg.sender << ": " << msg.content << std::endl;
                }
                break;
                
            case ChatMessage::PRIVATE_MSG:
                if (msg.sender != username_) {
                    std::cout << "[私聊] " << msg.sender << " -> 你: " << msg.content << std::endl;
                } else {
                    std::cout << "[私聊] " << msg.content << std::endl;
                }
                break;
                
            case ChatMessage::ROOM_LIST:
            case ChatMessage::USER_LIST:
                std::cout << msg.content << std::endl;
                break;
        }
    }
    
    void handle_command(const std::string& command) {
        std::vector<std::string> parts;
        std::string current;
        
        for (char c : command) {
            if (c == ' ') {
                if (!current.empty()) {
                    parts.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty()) {
            parts.push_back(current);
        }
        
        if (parts.empty()) return;
        
        std::string cmd = parts[0];
        
        if (cmd == "/help") {
            show_help();
        } else if (cmd == "/join" && parts.size() > 1) {
            // 加入房间
            ChatMessage msg;
            msg.type = ChatMessage::ROOM_JOIN;
            msg.sender = username_;
            msg.receiver = parts[1];
            send_message(msg);
            current_room_ = parts[1];
            std::cout << "正在加入房间: " << parts[1] << std::endl;
        } else if (cmd == "/msg" && parts.size() > 2) {
            // 私聊
            ChatMessage msg;
            msg.type = ChatMessage::PRIVATE_MSG;
            msg.sender = username_;
            msg.receiver = parts[1];
            
            // 组合消息内容
            msg.content = "";
            for (size_t i = 2; i < parts.size(); ++i) {
                if (i > 2) msg.content += " ";
                msg.content += parts[i];
            }
            
            send_message(msg);
        } else if (cmd == "/users") {
            // 获取用户列表
            ChatMessage msg;
            msg.type = ChatMessage::USER_LIST;
            send_message(msg);
        } else if (cmd == "/rooms") {
            // 获取房间列表
            ChatMessage msg;
            msg.type = ChatMessage::ROOM_LIST;
            send_message(msg);
        } else if (cmd == "/quit") {
            disconnect();
        } else {
            std::cout << "未知命令: " << cmd << std::endl;
            show_help();
        }
    }
    
    void show_help() {
        std::cout << "\n=== 聊天客户端帮助 ===" << std::endl;
        std::cout << "命令列表:" << std::endl;
        std::cout << "  /help          - 显示帮助信息" << std::endl;
        std::cout << "  /join <房间>   - 加入指定房间" << std::endl;
        std::cout << "  /msg <用户> <消息> - 发送私聊消息" << std::endl;
        std::cout << "  /users         - 显示在线用户列表" << std::endl;
        std::cout << "  /rooms         - 显示房间列表" << std::endl;
        std::cout << "  /quit          - 退出聊天" << std::endl;
        std::cout << "\n直接输入文字发送到当前房间" << std::endl;
        std::cout << "当前房间: " << (current_room_.empty() ? "未加入" : current_room_) << std::endl;
        std::cout << "========================\n" << std::endl;
    }
};

int main() {
    try {
        std::cout << "=== Boost.Asio 聊天客户端 ===" << std::endl;
        
        std::string host, port, username;
        
        std::cout << "服务器地址 (默认 localhost): ";
        std::getline(std::cin, host);
        if (host.empty()) host = "localhost";
        
        std::cout << "服务器端口 (默认 8080): ";
        std::getline(std::cin, port);
        if (port.empty()) port = "8080";
        
        std::cout << "用户名: ";
        std::getline(std::cin, username);
        if (username.empty()) {
            std::cout << "用户名不能为空" << std::endl;
            return 1;
        }
        
        asio::io_context io;
        ChatClient client(io);
        
        // 在单独线程中运行 io_context
        std::thread io_thread([&io]() {
            io.run();
        });
        
        // 连接到服务器
        client.connect(host, port, username);
        
        // 主线程处理用户输入
        std::string input;
        while (client.is_connected() && std::getline(std::cin, input)) {
            client.send_text_message(input);
        }
        
        client.disconnect();
        io.stop();
        
        if (io_thread.joinable()) {
            io_thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "客户端异常: " << e.what() << std::endl;
    }
    
    return 0;
}