#include "message/MessageTypes.hpp"
#include <cstring>
#include <chrono>
#include <iostream>

namespace savior_server {
namespace message {

// MessageHeader 实现
MessageHeader::MessageHeader() 
    : magic(0x53415649), length(sizeof(MessageHeader)), type(0), version(1), sequence(0) {
}

MessageHeader::MessageHeader(MessageType msgType, uint32_t msgLength)
    : magic(0x53415649), length(msgLength), type(static_cast<uint16_t>(msgType)), version(1), sequence(0) {
}

std::vector<uint8_t> MessageHeader::Serialize() const {
    std::vector<uint8_t> result(sizeof(MessageHeader));
    std::memcpy(result.data(), this, sizeof(MessageHeader));
    return result;
}

bool MessageHeader::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(MessageHeader)) {
        return false;
    }
    std::memcpy(this, data.data(), sizeof(MessageHeader));
    return magic == 0x53415649;
}

// Message 实现
Message::Message(MessageType type) {
    m_header.type = static_cast<uint16_t>(type);
    m_header.length = sizeof(MessageHeader);
}

MessageType Message::GetType() const {
    return static_cast<MessageType>(m_header.type);
}

const MessageHeader& Message::GetHeader() const {
    return m_header;
}

// TextMessage 实现
TextMessage::TextMessage() : Message(MessageType::CUSTOM) {
}

TextMessage::TextMessage(const std::string& text) : Message(MessageType::CUSTOM), m_text(text) {
}

const std::string& TextMessage::GetText() const {
    return m_text;
}

void TextMessage::SetText(const std::string& text) {
    m_text = text;
}

std::vector<uint8_t> TextMessage::Serialize() const {
    // 序列化消息头
    auto header_data = m_header.Serialize();
    
    // 序列化文本内容
    std::vector<uint8_t> result;
    result.reserve(sizeof(MessageHeader) + m_text.size());
    
    // 更新长度
    MessageHeader updated_header = m_header;
    updated_header.length = sizeof(MessageHeader) + static_cast<uint32_t>(m_text.size());
    auto updated_header_data = updated_header.Serialize();
    
    result.insert(result.end(), updated_header_data.begin(), updated_header_data.end());
    result.insert(result.end(), m_text.begin(), m_text.end());
    
    return result;
}

bool TextMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(MessageHeader)) {
        return false;
    }
    
    // 反序列化消息头
    if (!m_header.Deserialize(data)) {
        return false;
    }
    
    // 检查数据长度
    if (data.size() != m_header.length) {
        return false;
    }
    
    // 提取文本内容
    if (m_header.length > sizeof(MessageHeader)) {
        size_t text_size = m_header.length - sizeof(MessageHeader);
        m_text.assign(data.begin() + sizeof(MessageHeader), data.begin() + sizeof(MessageHeader) + text_size);
    } else {
        m_text.clear();
    }
    
    return true;
}

// HeartbeatMessage 实现
HeartbeatMessage::HeartbeatMessage() : Message(MessageType::SYSTEM) {
    m_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

HeartbeatMessage::HeartbeatMessage(uint64_t timestamp) : Message(MessageType::SYSTEM), m_timestamp(timestamp) {
}

uint64_t HeartbeatMessage::GetTimestamp() const {
    return m_timestamp;
}

void HeartbeatMessage::SetTimestamp(uint64_t timestamp) {
    m_timestamp = timestamp;
}

std::vector<uint8_t> HeartbeatMessage::Serialize() const {
    // 序列化消息头
    MessageHeader updated_header = m_header;
    updated_header.length = sizeof(MessageHeader) + sizeof(uint64_t);
    auto header_data = updated_header.Serialize();
    
    std::vector<uint8_t> result;
    result.reserve(sizeof(MessageHeader) + sizeof(uint64_t));
    
    result.insert(result.end(), header_data.begin(), header_data.end());
    
    // 序列化时间戳
    const uint8_t* timestamp_ptr = reinterpret_cast<const uint8_t*>(&m_timestamp);
    result.insert(result.end(), timestamp_ptr, timestamp_ptr + sizeof(uint64_t));
    
    return result;
}

bool HeartbeatMessage::Deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(MessageHeader) + sizeof(uint64_t)) {
        return false;
    }
    
    // 反序列化消息头
    if (!m_header.Deserialize(data)) {
        return false;
    }
    
    // 检查数据长度
    if (data.size() != m_header.length || m_header.length != sizeof(MessageHeader) + sizeof(uint64_t)) {
        return false;
    }
    
    // 反序列化时间戳
    std::memcpy(&m_timestamp, data.data() + sizeof(MessageHeader), sizeof(uint64_t));
    
    return true;
}

// MessageFactory 实现
std::unique_ptr<Message> MessageFactory::CreateMessage(MessageType type) {
    switch (type) {
        case MessageType::SYSTEM:
            return std::make_unique<HeartbeatMessage>();
        case MessageType::AUTH:
        case MessageType::GAME:
        case MessageType::CUSTOM:
            return std::make_unique<TextMessage>();
        default:
            return std::make_unique<HeartbeatMessage>();
    }
}

std::unique_ptr<Message> MessageFactory::DeserializeMessage(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(MessageHeader)) {
        return nullptr;
    }
    
    MessageHeader header;
    if (!header.Deserialize(data)) {
        return nullptr;
    }
    
    auto message = CreateMessage(static_cast<MessageType>(header.type));
    if (message && message->Deserialize(data)) {
        return message;
    }
    
    return nullptr;
}

} // namespace message
} // namespace savior_server