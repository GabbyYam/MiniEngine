#include "Scene/Component/Component.hpp"
#include "Scene/Entity/Entity.hpp"
#include "UUID.hpp"
#include <Scene/Scene.hpp>
#include <entt/entity/entity.hpp>
#include <stdint.h>
#include <vector>

namespace suplex {
    Entity Scene::CreateEntity(std::string const& name)
    {
        Entity e(m_Registry.create(), this);
        e.AddComponent<IDComponent>();
        e.AddComponent<TagComponent>(name);
        e.AddComponent<TransformComponent>();
        return e;
    }

    // Entity Scene::CreateEntityWithUUID(std::string const& name, UUID uuid)
    // {
    //     Entity e(m_Registry.create(), this);
    //     e.AddComponent<IDComponent>(uuid);
    //     e.AddComponent<TagComponent>(name);
    //     e.AddComponent<TransformComponent>();
    //     return e;
    // }

    Entity Scene::GetActiveEntity()
    {
        auto e = m_Registry.view<TransformComponent>();

        return e.empty() ? Entity(entt::null, this) : Entity(*e.begin(), this);
    }

}  // namespace suplex