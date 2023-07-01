#pragma once

#include "Render/RenderPass/RenderPass.hpp"
namespace suplex {
    class PostprocessPass : public RenderPass {
    public:
        virtual void Render(const std::shared_ptr<Camera>            camera,
                            const std::shared_ptr<Scene>             scene,
                            const std::shared_ptr<GraphicsContext>   graphicsContext,
                            const std::shared_ptr<PrecomputeContext> context) override
        {
        }

    private:
    };
}  // namespace suplex