// #pragma once

// #include "Camera.hpp"
// #include <glm/ext/matrix_transform.hpp>
// namespace suplex {
//     class LightCamera : public Camera {
//     public:
//         LightCamera(float fov, float nearClip, float farClip) : Camera(fov, nearClip, farClip)
//         {
//             m_Position.y = 5;
//             LookAtWorldCenter();
//             RecalculateOrthoProjection();
//         };

//         void RecalculateOrthoProjection() { m_OrthoProjection = glm::ortho<float>(-10, 10, -10, 10, m_NearClip, m_FarClip); }

//         void LookAtWorldCenter() { m_View = glm::lookAt(m_Position - m_Forward, glm::vec3(0.0f), glm::vec3(0, 1, 0)); }
//     };
// }  // namespace suplex