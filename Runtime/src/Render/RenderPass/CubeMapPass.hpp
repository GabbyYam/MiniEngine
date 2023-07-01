#pragma once
#include "Render/Config/Config.hpp"
#include "Render/RenderPass/RenderPass.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "Render/Geometry/Shape/Shape.hpp"

namespace suplex {
    class CubeMapPass : public RenderPass {
        virtual void Render(const std::shared_ptr<Camera>            camera,
                            const std::shared_ptr<Scene>             scene,
                            const std::shared_ptr<GraphicsContext>   graphicsContext,
                            const std::shared_ptr<PrecomputeContext> context) override
        {
            // render to custom framebuffer
            // ------
            glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer->GetID());
            // glClearColor(.6f, .7f, .9f, 1.0f);
            // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            auto view = camera->GetView();
            auto proj = camera->GetProjection();

            view = mat4(mat3(view));

            auto& shader = m_Shaders[0];
            glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
            shader->Bind();          // remove translation from the view matrix
            shader->SetMaterix4("view", glm::value_ptr(view));
            shader->SetMaterix4("proj", glm::value_ptr(proj));

            // skybox cube
            // bake::Cube(shader);
            utils::RenderCube(shader);
            shader->Unbind();

            // Return to default framebuffer
            glDepthFunc(GL_LESS);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    };
}  // namespace suplex