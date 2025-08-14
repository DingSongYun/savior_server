// 聊天服务器 - 服务器端实现
// 编译命令: g++ -std=c++17 chat_server.cpp -lboost_system -pthread -o chat_server

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <mutex>
#include <algorithm>
#include <chrono>
#include <ctime>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// 消息类型定义
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
    std::string receiver;  // 私聊接收者或房间名
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

// 前向声明
class ChatRoom;
class ChatServer;

// 聊天客户端连接类
class ChatClient : public std::enable_shared_from_this<ChatClient> {
private:
    tcp::socket socket_;
    ChatServer& server_;
    std::string username_;
    std::string current_room_;
    std::array<char, 1024> read_buffer_;
    std::deque<std::string> write_queue_;
    asio::io_context::strand strand_;
    
public:
    ChatClient(tcp::socket socket, ChatServer& server);
    
    void start();
    void send_message(const ChatMessage& msg);
    void disconnect();
    
    const std::string& username() const { return username_; }
    const std::string& current_room() const { return current_room_; }
    void set_room(const std::string& room) { current_room_ = room; }
    
private:
    void start_read();
    void handle_message(const std::string& data);
    void start_write();
    std::string get_timestamp();
};

// 聊天房间类
class ChatRoom {
private:
    std::string name_;
    std::unordered_set<std::shared_ptr<ChatClient>> clients_;
    std::deque<ChatMessage> message_history_;
    static const size_t MAX_HISTORY = 100;
    
public:
    ChatRoom(const std::string& name) : name_(name) {}
    
    void add_client(std::shared_ptr<ChatClient> client) {
        clients_.insert(client);
        
        // 发送历史消息
        for (const auto& msg : message_history_) {
            client->send_message(msg);
        }
        
        // 通知其他用户
        ChatMessage join_msg;
        join_msg.type = ChatMessage::JOIN;
        join_msg.sender = client->username();
        join_msg.receiver = name_;
        join_msg.content = client->username() + " 加入了房间";
        join_msg.timestamp = get_timestamp();
        
        broadcast_message(join_msg);
    }
    
    void remove_client(std::shared_ptr<ChatClient> client) {
        clients_.erase(client);
        
        // 通知其他用户
        ChatMessage leave_msg;
        leave_msg.type = ChatMessage::LEAVE;
        leave_msg.sender = client->username();
        leave_msg.receiver = name_;
        leave_msg.content = client->username() + " 离开了房间";
        leave_msg.timestamp = get_timestamp();
        
        broadcast_message(leave_msg);
    }
    
    void broadcast_message(const ChatMessage& msg) {
        // 添加到历史记录
        message_history_.push_back(msg);
        if (message_history_.size() > MAX_HISTORY) {
            message_history_.pop_front();
        }
        
        // 广播给所有客户端
        for (auto& client : clients_) {
            client->send_message(msg);
        }
    }
    
    size_t client_count() const { return clients_.size(); }
    const std::string& name() const { return name_; }
    
    std::vector<std::string> get_user_list() const {
        std::vector<std::string> users;
        for (const auto& client : clients_) {
            users.push_back(client->username());
        }
        return users;
    }
    
private:
    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto tm = *std::localtime(&time_t);
        
        char buffer[100];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
        return std::string(buffer);
    }
};

// 聊天服务器类
class ChatServer {
private:
    asio::io_context& io_;
    tcp::acceptor acceptor_;
    std::unordered_map<std::string, std::shared_ptr<ChatRoom>> rooms_;
    std::unordered_map<std::string, std::shared_ptr<ChatClient>> clients_;
    std::mutex clients_mutex_;
    std::mutex rooms_mutex_;
    
public:
    ChatServer(asio::io_context& io, short port)
        : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
        
        // 创建默认房间
        create_room("大厅");
        create_room("技术讨论");
        create_room("闲聊");
    }
    
    void start() {
        std::cout << "聊天服务器启动，监听端口: " << acceptor_.local_endpoint().port() << std::endl;
        start_accept();
    }
    
    void add_client(const std::string& username, std::shared_ptr<ChatClient> client) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_[username] = client;
        std::cout << "用户 " << username << " 连接到服务器" << std::endl;
    }
    
    void remove_client(const std::string& username) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_.erase(username);
        std::cout << "用户 " << username << " 断开连接" << std::endl;
    }
    
    std::shared_ptr<ChatClient> find_client(const std::string& username) {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        auto it = clients_.find(username);
        return (it != clients_.end()) ? it->second : nullptr;
    }
    
    std::shared_ptr<ChatRoom> get_room(const std::string& room_name) {
        std::lock_guard<std::mutex> lock(rooms_mutex_);
        auto it = rooms_.find(room_name);
        return (it != rooms_.end()) ? it->second : nullptr;
    }
    
    std::shared_ptr<ChatRoom> create_room(const std::string& room_name) {
        std::lock_guard<std::mutex> lock(rooms_mutex_);
        auto room = std::make_shared<ChatRoom>(room_name);
        rooms_[room_name] = room;
        std::cout << "创建房间: " << room_name << std::endl;
        return room;
    }
    
    std::vector<std::string> get_room_list() {
        std::lock_guard<std::mutex> lock(rooms_mutex_);
        std::vector<std::string> room_list;
        for (const auto& pair : rooms_) {
            room_list.push_back(pair.first + " (" + std::to_string(pair.second->client_count()) + ")");
        }
        return room_list;
    }
    
    std::vector<std::string> get_user_list() {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        std::vector<std::string> user_list;
        for (const auto& pair : clients_) {
            user_list.push_back(pair.first);
        }
        return user_list;
    }
    
private:
    void start_accept() {
        acceptor_.async_accept(
            [this](const boost::system::error_code& ec, tcp::socket socket) {
                if (!ec) {
                    auto client = std::make_shared<ChatClient>(std::move(socket), *this);
                    client->start();
                }
                start_accept();
            });
    }
};

// ChatClient 实现
ChatClient::ChatClient(tcp::socket socket, ChatServer& server)
    : socket_(std::move(socket)), server_(server), strand_(socket_.get_executor()) {}

void ChatClient::start() {
    start_read();
}

void ChatClient::send_message(const ChatMessage& msg) {
    strand_.post([this, msg, self = shared_from_this()]() {
        bool write_in_progress = !write_queue_.empty();
        write_queue_.push_back(msg.serialize());
        if (!write_in_progress) {
            start_write();
        }
    });
}

void ChatClient::disconnect() {
    if (!username_.empty()) {
        // 离开当前房间
        if (!current_room_.empty()) {
            auto room = server_.get_room(current_room_);
            if (room) {
                room->remove_client(shared_from_this());
            }
        }
        
        // 从服务器移除
        server_.remove_client(username_);
    }
    
    socket_.close();
}

void ChatClient::start_read() {
    socket_.async_read_some(asio::buffer(read_buffer_),
        strand_.wrap([this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t length) {
            if (!ec) {
                std::string data(read_buffer_.data(), length);
                handle_message(data);
                start_read();
            } else {
                disconnect();
            }
        }));
}

void ChatClient::handle_message(const std::string& data) {
    auto msg = ChatMessage::deserialize(data);
    
    switch (msg.type) {
        case ChatMessage::JOIN: {
            // 用户登录
            if (username_.empty()) {
                username_ = msg.sender;
                server_.add_client(username_, shared_from_this());
                
                // 自动加入大厅
                auto lobby = server_.get_room("大厅");
                if (lobby) {
                    current_room_ = "大厅";
                    lobby->add_client(shared_from_this());
                }
                
                // 发送房间列表
                ChatMessage room_list_msg;
                room_list_msg.type = ChatMessage::ROOM_LIST;
                room_list_msg.content = "可用房间:\n";
                for (const auto& room : server_.get_room_list()) {
                    room_list_msg.content += "- " + room + "\n";
                }
                send_message(room_list_msg);
            }
            break;
        }
        
        case ChatMessage::MESSAGE: {
            // 房间消息
            if (!current_room_.empty()) {
                auto room = server_.get_room(current_room_);
                if (room) {
                    ChatMessage room_msg = msg;
                    room_msg.sender = username_;
                    room_msg.receiver = current_room_;
                    room_msg.timestamp = get_timestamp();
                    room->broadcast_message(room_msg);
                }
            }
            break;
        }
        
        case ChatMessage::PRIVATE_MSG: {
            // 私聊消息
            auto target_client = server_.find_client(msg.receiver);
            if (target_client) {
                ChatMessage private_msg = msg;
                private_msg.sender = username_;
                private_msg.timestamp = get_timestamp();
                target_client->send_message(private_msg);
                
                // 给发送者确认
                ChatMessage confirm_msg = private_msg;
                confirm_msg.content = "[私聊给 " + msg.receiver + "] " + msg.content;
                send_message(confirm_msg);
            } else {
                ChatMessage error_msg;
                error_msg.type = ChatMessage::MESSAGE;
                error_msg.content = "用户 " + msg.receiver + " 不在线";
                send_message(error_msg);
            }
            break;
        }
        
        case ChatMessage::ROOM_JOIN: {
            // 加入房间
            auto new_room = server_.get_room(msg.receiver);
            if (!new_room) {
                new_room = server_.create_room(msg.receiver);
            }
            
            // 离开当前房间
            if (!current_room_.empty()) {
                auto old_room = server_.get_room(current_room_);
                if (old_room) {
                    old_room->remove_client(shared_from_this());
                }
            }
            
            // 加入新房间
            current_room_ = msg.receiver;
            new_room->add_client(shared_from_this());
            break;
        }
        
        case ChatMessage::USER_LIST: {
            // 发送用户列表
            ChatMessage user_list_msg;
            user_list_msg.type = ChatMessage::USER_LIST;
            user_list_msg.content = "在线用户:\n";
            for (const auto& user : server_.get_user_list()) {
                user_list_msg.content += "- " + user + "\n";
            }
            send_message(user_list_msg);
            break;
        }
    }
}

void ChatClient::start_write() {
    if (write_queue_.empty()) return;
    
    asio::async_write(socket_, asio::buffer(write_queue_.front()),
        strand_.wrap([this, self = shared_from_this()](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                write_queue_.pop_front();
                if (!write_queue_.empty()) {
                    start_write();
                }
            } else {
                disconnect();
            }
        }));
}

std::string ChatClient::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buffer);
}

int main() {
    try {
        asio::io_context io;
        
        std::cout << "请输入服务器端口 (默认 8080): ";
        std::string port_str;
        std::getline(std::cin, port_str);
        short port = port_str.empty() ? 8080 : static_cast<short>(std::stoi(port_str));
        
        ChatServer server(io, port);
        server.start();
        
        std::cout << "聊天服务器运行中... 按 Ctrl+C 停止" << std::endl;
        std::cout << "客户端可以使用 telnet localhost " << port << " 连接" << std::endl;
        
        io.run();
        
    } catch (const std::exception& e) {
        std::cerr << "服务器异常: " << e.what() << std::endl;
    }
    
    return 0;
}