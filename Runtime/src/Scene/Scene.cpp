#include "Scene/Component/Component.hpp"
#include "Scene/Entity/Entity.hpp"
#include <Scene/Scene.hpp>
#include <entt/entity/entity.hpp>
#include <vector>

namespace suplex {
    Entity Scene::CreateEntity(std::string const& name)
    {
        Entity e(m_Registry.create(), this);
        e.AddComponent<TagComponent>(name);
        e.AddComponent<TransformComponent>();
        return e;
    }

    Entity Scene::GetActiveEntity()
    {
        auto e = m_Registry.view<TransformComponent>();

        return e.empty() ? Entity(entt::null, this) : Entity(*e.begin(), this);
    }

}  // namespace suplex