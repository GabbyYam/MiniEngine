#pragma once

#include "Render/Camera/Camera.hpp"
#include "Render/Renderer.hpp"
#include "Scene/Entity/Entity.hpp"
#include <entt/entity/entity.hpp>
#include <memory>
#include <stdint.h>
namespace suplex {

    struct RuntimeContext
    {
        std::shared_ptr<Renderer> renderer;
        std::shared_ptr<Camera>   camera;
    };

    class Panel {
    public:
        Panel() = default;

        virtual void OnEvent() {}
        virtual void OnUpdate(float ts) {}
        virtual void OnUIRender() {}
        virtual void SetContext(const std::shared_ptr<RuntimeContext> context) { m_Context = context; }

    protected:
        std::shared_ptr<RuntimeContext> m_Context = nullptr;
    };
}  // namespace suplex