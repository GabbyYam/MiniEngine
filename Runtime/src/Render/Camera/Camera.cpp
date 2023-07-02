#include "Camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "InputSystem/Input.h"
#include "InputSystem/KeyCodes.h"

namespace suplex {

    Camera::Camera(float fov, float nearClip, float farClip, ProjectionType projectionType)
        : m_VerticalFOV(fov), m_NearClip(nearClip), m_FarClip(farClip), m_ProjectionType(projectionType)
    {
        m_Position = {0.f, 5.f, 15.f};
        m_Forward  = {0.f, -.2f, -1.f};

        RecalculateView();
        RecalculateProjection();
    }

    void Camera::OnResize(uint32_t w, uint32_t h)
    {
        if (w == m_ViewportWidth && h == m_ViewportHeight) return;
        m_ViewportWidth  = w;
        m_ViewportHeight = h;
        RecalculateProjection();
    }

    bool Camera::OnUpdate(float ts)
    {
        glm::vec2 mousePos  = Input::GetMousePosition();
        glm::vec2 delta     = (mousePos - m_LastMousePosition) * 0.002f;
        m_LastMousePosition = mousePos;

        if (!Input::IsMouseButtonDown(MouseButton::Right)) {
            Input::SetCursorMode(CursorMode::Normal);
            return false;
        }

        Input::SetCursorMode(CursorMode::Locked);

        bool moved = false;

        constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
        glm::vec3           rightDirection = glm::normalize(glm::cross(m_Forward, upDirection));

        float speed = 5.0f;

        // Movement
        if (Input::IsKeyDown(KeyCode::W)) {
            m_Position += m_Forward * speed * ts;
            moved = true;
        }
        else if (Input::IsKeyDown(KeyCode::S)) {
            m_Position -= m_Forward * speed * ts;
            moved = true;
        }
        if (Input::IsKeyDown(KeyCode::A)) {
            m_Position -= rightDirection * speed * ts;
            moved = true;
        }
        else if (Input::IsKeyDown(KeyCode::D)) {
            m_Position += rightDirection * speed * ts;
            moved = true;
        }
        if (Input::IsKeyDown(KeyCode::Q)) {
            m_Position -= upDirection * speed * ts;
            moved = true;
        }
        else if (Input::IsKeyDown(KeyCode::E)) {
            m_Position += upDirection * speed * ts;
            moved = true;
        }

        // Rotation
        if (delta.x != 0.0f || delta.y != 0.0f) {
            float pitchDelta = delta.y * GetRotationSpeed();
            float yawDelta   = delta.x * GetRotationSpeed();

            glm::quat q = glm::normalize(
                glm::cross(glm::angleAxis(-pitchDelta, rightDirection), glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
            m_Forward = glm::rotate(q, m_Forward);
            moved     = true;
        }

        if (moved) { RecalculateView(); }

        return moved;
    }

    float Camera::GetRotationSpeed() { return 0.3f; }

    void Camera::RecalculateView()
    {
        m_View = glm::lookAt(m_Position, m_Position + m_Forward, glm::vec3(0, 1, 0));
        // m_InverseView = glm::inverse(m_View);
    }

    void Camera::RecalculateProjection()
    {
        switch (m_ProjectionType) {
            case ProjectionType::Perspective:
                m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight,
                                                   m_NearClip, m_FarClip);
                break;
            case ProjectionType::Orthogonal: m_Projection = glm::ortho<float>(-10, 10, -10, 10, m_NearClip, m_FarClip); break;
        }
    }
}  // namespace suplex