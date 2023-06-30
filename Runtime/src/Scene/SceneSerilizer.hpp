#pragma once
#include "Scene/Scene.hpp"
#include "imgui_internal.h"
#include <memory>
#include <yaml-cpp/yaml.h>
#include <Scene/Entity/Entity.hpp>
#include <string_view>

namespace suplex {
    class SceneSerializer {
    public:
        SceneSerializer(const std::shared_ptr<Scene> scene) : m_Scene(scene) {}

        void Serialize(std::string_view path);
        bool Deserialize(std::string_view path);

        void SerializeEntity(YAML::Emitter& out, Entity entity);
        void SetContext(const std::shared_ptr<Scene> scene) { m_Scene = scene; }

    private:
        std::shared_ptr<Scene> m_Scene = nullptr;
    };
}  // namespace suplex