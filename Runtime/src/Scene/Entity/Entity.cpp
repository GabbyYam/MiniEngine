#include "Scene/Scene.hpp"
#include <Scene/Entity/Entity.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entt.hpp>
#include <vector>

namespace suplex {
    Entity::Entity(entt::entity entity, Scene* scene) : m_EntityHandle(entity), m_Scene(scene) {}
}  // namespace suplex