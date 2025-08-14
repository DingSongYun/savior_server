#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <cstdint>

namespace savior_server {
namespace entity {

// 类型定义
using EntityId = uint64_t;
using ComponentId = std::type_index;

/**
 * @brief 组件接口
 */
class IComponent {
public:
    virtual ~IComponent() = default;
};

/**
 * @brief 实体类
 * 
 * 实体组件系统中的实体，可以添加、移除和获取组件
 */
class Entity {
public:
    explicit Entity(EntityId id);
    ~Entity();

    /**
     * @brief 获取实体ID
     */
    EntityId GetId() const;

    /**
     * @brief 添加组件
     */
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of_v<IComponent, T>, "T must inherit from IComponent");
        
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = component.get();
        
        ComponentId id = std::type_index(typeid(T));
        m_components[id] = std::move(component);
        
        return ptr;
    }

    /**
     * @brief 移除组件
     */
    template<typename T>
    void RemoveComponent() {
        ComponentId id = std::type_index(typeid(T));
        m_components.erase(id);
    }

    /**
     * @brief 获取组件
     */
    template<typename T>
    T* GetComponent() {
        ComponentId id = std::type_index(typeid(T));
        auto it = m_components.find(id);
        if (it != m_components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    /**
     * @brief 检查是否有指定组件
     */
    template<typename T>
    bool HasComponent() const {
        ComponentId id = std::type_index(typeid(T));
        return m_components.find(id) != m_components.end();
    }

    /**
     * @brief 设置实体激活状态
     */
    void SetActive(bool active);

    /**
     * @brief 检查实体是否激活
     */
    bool IsActive() const;

private:
    EntityId m_id;
    bool m_active;
    std::unordered_map<ComponentId, std::unique_ptr<IComponent>> m_components;
};

/**
 * @brief 实体管理器
 * 
 * 负责实体的创建、销毁和管理
 */
class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    /**
     * @brief 创建实体
     */
    Entity* CreateEntity();

    /**
     * @brief 销毁实体
     */
    void DestroyEntity(EntityId id);

    /**
     * @brief 获取实体
     */
    Entity* GetEntity(EntityId id);

    /**
     * @brief 获取所有实体
     */
    std::vector<Entity*> GetAllEntities();

    /**
     * @brief 清除所有实体
     */
    void Clear();

    /**
     * @brief 获取实体数量
     */
    size_t GetEntityCount() const;

private:
    EntityId m_nextId;
    std::unordered_map<EntityId, std::unique_ptr<Entity>> m_entities;
};

} // namespace entity
} // namespace savior_server