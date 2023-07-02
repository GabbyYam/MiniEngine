#pragma once

#include "Render/Shader/Shader.hpp"
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include "glm/glm.hpp"
namespace suplex {
    enum class Shape { Cube, Quad, Sphere };
    enum class QuadRenderSpecification { Screen, Texture2D };

    namespace utils {
        void RenderQuad(const std::shared_ptr<Shader> shader, QuadRenderSpecification spec);
        void RenderCube(const std::shared_ptr<Shader> shader);
        void RenderSphere(const std::shared_ptr<Shader> shader);
        void RenderGird(const std::shared_ptr<Shader> shader);

    }  // namespace utils
}  // namespace suplex
