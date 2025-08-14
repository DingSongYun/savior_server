#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstdint>

namespace savior_server {
namespace message {

/**
 * @brief 消息类型枚举
 */
enum class MessageType : uint16_t {
    SYSTEM = 0x0000,        // 系统消息
    AUTH = 0x0100,          // 认证消息
    GAME = 0x0200,          // 游戏消息
    CUSTOM = 0x8000         // 自定义消息
};

/**
 * @brief 消息头结构
 * 固定16字节
 */
struct MessageHeader {
    uint32_t magic;         // 魔数 0x53415649 ("SAVI")
    uint32_t length;        // 消息总长度（包含头部）
    uint16_t type;          // 消息类型
    uint16_t version;       // 协议版本
    uint32_t sequence;      // 序列号
    
    MessageHeader();
    MessageHeader(MessageType msgType, uint32_t msgLength);
    
    // 序列化和反序列化
    std::vector<uint8_t> Serialize() const;
    bool Deserialize(const std::vector<uint8_t>& data);
};

/**
 * @brief 消息基类
 */
class Message {
public:
    Message(MessageType type);
    virtual ~Message() = default;

    /**
     * @brief 获取消息类型
     */
    MessageType GetType() const;

    /**
     * @brief 获取消息头
     */
    const MessageHeader& GetHeader() const;

    /**
     * @brief 序列化消息
     */
    virtual std::vector<uint8_t> Serialize() const = 0;

    /**
     * @brief 反序列化消息
     */
    virtual bool Deserialize(const std::vector<uint8_t>& data) = 0;

protected:
    MessageHeader m_header;
};

/**
 * @brief 文本消息类
 */
class TextMessage : public Message {
public:
    TextMessage();
    explicit TextMessage(const std::string& text);

    const std::string& GetText() const;
    void SetText(const std::string& text);

    std::vector<uint8_t> Serialize() const override;
    bool Deserialize(const std::vector<uint8_t>& data) override;

private:
    std::string m_text;
};

/**
 * @brief 心跳消息类
 */
class HeartbeatMessage : public Message {
public:
    HeartbeatMessage();
    explicit HeartbeatMessage(uint64_t timestamp);

    uint64_t GetTimestamp() const;
    void SetTimestamp(uint64_t timestamp);

    std::vector<uint8_t> Serialize() const override;
    bool Deserialize(const std::vector<uint8_t>& data) override;

private:
    uint64_t m_timestamp;
};

/**
 * @brief 消息工厂类
 */
class MessageFactory {
public:
    /**
     * @brief 创建消息
     */
    static std::unique_ptr<Message> CreateMessage(MessageType type);

    /**
     * @brief 从数据反序列化消息
     */
    static std::unique_ptr<Message> DeserializeMessage(const std::vector<uint8_t>& data);
};

} // namespace message
} // namespace savior_server