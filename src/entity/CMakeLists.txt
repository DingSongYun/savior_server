# Entity模块
set(ENTITY_SOURCES
    Entity.cpp
)

set(ENTITY_HEADERS
    ${CMAKE_SOURCE_DIR}/include/entity/Entity.hpp
)

add_library(savior_entity STATIC
    ${ENTITY_SOURCES}
    ${ENTITY_HEADERS}
)

# 设置包含目录
target_include_directories(savior_entity
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/libs/boost
)

# 链接依赖
target_link_libraries(savior_entity
    PUBLIC
    savior_core
    savior_message
    Boost::system
)