#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include <forward_list>
#include <memory>
#include "entt/entt.hpp"
#include <Scene/Scene.hpp>
#include <vector>

namespace suplex {

    class Entity {
    public:
        Entity(entt::entity entity, Scene* scene);

        template <class T>
        T& GetComponent()
        {
            assert(HasComponent<T>());
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template <class... Ts>
        bool HasComponent()
        {
            return m_Scene->m_Registry.all_of<Ts...>(m_EntityHandle);
        }

        template <class T, class... Args>
        void AddComponent(Args&&... args)
        {
            assert(!HasComponent<T>());
            m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template <class T>
        void RemoveComponent()
        {
            assert(HasComponent<T>());
            m_Scene->m_Registry.remove<T>();
        }

        operator bool() { return m_EntityHandle != entt::null; }

    private:
        entt::entity m_EntityHandle{entt::null};
        Scene*       m_Scene = nullptr;
    };

    template <class... Ts>
    [[nodiscard("")]] std::vector<Entity> MakeEntities(auto& view, Scene* scene)
    {
        std::vector<Entity> ret;
        for (auto e : view) ret.emplace_back(view, scene);
        return ret;
    }
}  // namespace suplex