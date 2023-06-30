#include "SceneSerilizer.hpp"
#include "Scene/Component/Component.hpp"
#include "Scene/SceneSerilizer.hpp"
#include <entt/entity/entity.hpp>
#include <glm/fwd.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/exceptions.h>

namespace YAML {

    template <>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2) return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template <>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3) return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template <>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4) return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
}  // namespace YAML

namespace suplex {

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        out << YAML::BeginMap;  // Entity
        out << YAML::Key << "Entity" << YAML::Value << "1270230219381";

        if (entity.HasComponent<TagComponent>()) {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;  // TagComponent

            auto& tag = entity.GetComponent<TagComponent>().m_Tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;

            out << YAML::EndMap;  // TagComponent
        }

        if (entity.HasComponent<TransformComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;  // TransformComponent

            auto& tc = entity.GetComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.m_Translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.m_Rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.m_Scale;

            out << YAML::EndMap;  // TransformComponent
        }

        if (entity.HasComponent<MeshRendererComponent>()) {
            out << YAML::Key << "MeshRendererComponent";
            out << YAML::BeginMap;

            auto& mrc = entity.GetComponent<MeshRendererComponent>();
            out << YAML::Key << "Filepath" << YAML::Value << mrc.m_Model->GetFilePath();
            out << YAML::Key << "MaterialIndex" << YAML::Value << mrc.m_Model->GetMaterialIndex();

            out << YAML::EndMap;
        }
        out << YAML::EndMap;
    }

    void SceneSerializer::Serialize(std::string_view path)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        m_Scene->m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, m_Scene.get()};
            if (!entity) return;

            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(path.data());
        fout << out.c_str();
    }

    bool SceneSerializer::Deserialize(std::string_view path)
    {
        YAML::Node data;
        try {
            data = YAML::LoadFile(path.data());
        }
        catch (YAML::ParserException e) {
            return false;
        }

        if (!data["Scene"]) return false;

        std::string sceneName = data["Scene"].as<std::string>();

        auto entities = data["Entities"];
        if (entities) {
            for (auto entity : entities) {
                uint64_t uuid = entity["Entity"].as<uint64_t>();

                std::string name;

                auto tagComponent = entity["TagComponent"];
                if (tagComponent) name = tagComponent["Tag"].as<std::string>();

                Entity deserializedEntity = m_Scene->CreateEntity(name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent) {
                    // Entities always have transforms
                    auto& tc         = deserializedEntity.GetComponent<TransformComponent>();
                    tc.m_Translation = transformComponent["Translation"].as<glm::vec3>();
                    tc.m_Rotation    = transformComponent["Rotation"].as<glm::vec3>();
                    tc.m_Scale       = transformComponent["Scale"].as<glm::vec3>();
                }

                auto meshRendererComponent = entity["MeshRendererComponent"];
                if (meshRendererComponent) {
                    deserializedEntity.AddComponent<MeshRendererComponent>();
                    auto&       mrc              = deserializedEntity.GetComponent<MeshRendererComponent>();
                    std::string filepath         = meshRendererComponent["Filepath"].as<std::string>();
                    mrc.m_Model->m_FilePath      = filepath;
                    mrc.m_Model->m_MaterialIndex = meshRendererComponent["MaterialIndex"].as<uint32_t>();
                    mrc.m_Model->LoadModel();
                }
            }
        }
        return true;
    }
}  // namespace suplex
