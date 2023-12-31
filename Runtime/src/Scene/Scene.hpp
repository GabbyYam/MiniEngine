#pragma once

#include "Scene/Component/Component.hpp"
#include "entt/entt.hpp"
#include <glm/fwd.hpp>
#include <iterator>
#include <memory>
#include <stdint.h>
#include <vector>

namespace suplex {
    class Entity;

    class Scene {
    public:
        Scene() = default;
        ~Scene() {}

        Entity CreateEntity(std::string const& name = "");
        // Entity CreateEntityWithUUID(std::string const& name, UUID uuid){};

        template <class... Components>
        auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        // template <class... Components>
        // std::vector<Entity> GetEntities()
        // {
        //     std::vector<Entity> v;
        //     for (auto& e : m_Registry.view<Components...>()) v.emplace_back(e, this);
        //     return v;
        // }

        Entity GetActiveEntity();

    public:
        entt::registry m_Registry;

    private:
        uint32_t m_Width = 0, m_Height = 0;

        friend class Entity;
    };

}  // namespace suplex