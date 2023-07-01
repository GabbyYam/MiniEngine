#pragma once

#include "Render/Geometry/Mesh.hpp"
#include "Render/Geometry/Model.hpp"
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <Render/Material/Material.hpp>
#include <UUID.hpp>

namespace suplex {

    struct IDComponent
    {
        UUID ID;

        IDComponent()                   = default;
        IDComponent(const IDComponent&) = default;
    };

    struct TagComponent
    {
        std::string m_Tag;

        TagComponent() = default;
        TagComponent(std::string const& tag) : m_Tag(tag) {}
    };

    struct TransformComponent
    {
        auto GetTransform()
        {
            auto translate = glm::translate(glm::mat4(1.0f), m_Translation);
            auto rotation  = glm::toMat4(glm::quat(glm::radians(m_Rotation)));
            auto scale     = glm::scale(glm::mat4(1.0f), m_Scale);
            return translate * rotation * scale;
        }

        void SetTransform(const glm::mat4& transform)
        {
            auto quaternion = glm::quat();
            glm::decompose(transform, m_Scale, quaternion, m_Translation, m_Skew, m_Perspective);
            m_Rotation = glm::degrees(glm::eulerAngles(quaternion));
        }

        TransformComponent(glm::vec3 const& translation = glm::vec3{0.0f},
                           glm::vec3 const& rotation    = glm::vec3{0.0f},
                           glm::vec3 const& scale       = glm::vec3{1.0f})
            : m_Translation(translation), m_Rotation(rotation), m_Scale(scale)
        {
        }

        glm::vec3 m_Translation{0.0f};
        glm::vec3 m_Rotation{0.0f};
        glm::vec3 m_Scale{1.0f};
        glm::vec3 m_Skew{0.0f};
        glm::vec4 m_Perspective{0.0f};
    };

    struct MeshRendererComponent
    {
        std::shared_ptr<Model> m_Model = std::make_shared<Model>();

        MeshRendererComponent() = default;
        // MeshRendererComponent(const std::shared_ptr<Model> model) : m_Model(model) {}
        MeshRendererComponent(const Model& model) { m_Model = std::make_shared<Model>(model); }
    };

    struct MaterialComponent
    {
        std::shared_ptr<Material> m_Material = std::make_shared<Material>();
    };

    struct EnvironmentMapComponent
    {
    };

    struct LightComponent
    {
        enum class LightType {
            Point,
            Directional,
            Area,
        };

        glm::vec3 m_LightColor{1.0f};
        float     m_LightIntensity = 1.0f;
    };

}  // namespace suplex