#pragma once

#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <stdint.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace suplex {
    class Camera {
    public:
        Camera() = default;
        Camera(float fov, float nearClip, float farClip);
        virtual ~Camera() {}

        bool OnUpdate(float ts);
        void OnResize(uint32_t w, uint32_t h);

        auto& GetLastMousePosition() const { return m_LastMousePosition; }
        auto& GetPosition() { return m_Position; }
        auto& GetProjection() { return m_Projection; }
        auto& GetView() { return m_View; }
        auto& GetInverseProjection() { return m_InverseProjection; }
        auto& GetInverseView() { return m_InverseView; }
        auto& GetOrthoProjection() { return m_OrthoProjection; }

        auto& GetNearClip() { return m_NearClip; }
        auto& GetFarClip() { return m_FarClip; }
        auto& GetForward() { return m_Forward; }

    protected:
        virtual void  RecalculateView();
        virtual void  RecalculateProjection();
        virtual float GetRotationSpeed();

    protected:
        float     m_VerticalFOV = 45.0f, m_NearClip = 0.01f, m_FarClip = 1000.f;
        uint32_t  m_ViewportWidth = 800, m_ViewportHeight = 600;
        glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
        glm::vec3 m_Forward  = {0.0f, 0.0f, -1.0f};
        glm::vec2 m_LastMousePosition{0.0f, 0.0f};

        glm::mat4 m_Projection, m_InverseProjection;
        glm::mat4 m_OrthoProjection;

        glm::mat4 m_View, m_InverseView;
    };
}  // namespace suplex