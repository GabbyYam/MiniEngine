#pragma once

#include "Render/Buffer/Framebuffer.hpp"
#include "Render/Geometry/Model.hpp"
#include "Render/Geometry/Shape/Shape.hpp"
#include "Render/Postprocess/Noise.hpp"
#include "Render/RenderPass/RenderPass.hpp"
#include <memory>
#include <spdlog/spdlog.h>
namespace suplex {
    class SSAOPass : public RenderPass {
    public:
        SSAOPass()
        {
            auto  ssaoNoise  = noise.SSAONoise();
            auto  ssaoKernel = noise.SSAOKernel();
            void* data       = ssaoNoise.data();

            m_NoiseMap = std::make_shared<Texture2D>(data);

            auto shader = m_Shaders.emplace_back(std::make_shared<Shader>("quad.vert", "ssao.frag"));
            shader->Bind();
            for (int i = 0; i < 64; ++i)
                shader->SetFloat3(("samples[" + std::to_string(i) + "]"), glm::value_ptr(ssaoKernel[i]));
        }

        virtual void Render(const std::shared_ptr<Camera>            camera,
                            const std::shared_ptr<Scene>             scene,
                            const std::shared_ptr<GraphicsContext>   graphicsContext,
                            const std::shared_ptr<PrecomputeContext> context) override
        {
            m_Framebuffer->Bind();
            auto config = graphicsContext->config;
            auto shader = m_Shaders.back();
            shader->Bind();

            shader->SetMaterix4("projection", glm::value_ptr(camera->GetProjection()));
            shader->SetInt("enableSSAO", config->postprocessSetting.enableSSAO);
            shader->BindTexture("gPosition", graphicsContext->gPosition, 15, SamplerType::Texture2D);
            shader->BindTexture("gNormal", graphicsContext->gNormal, 14, SamplerType::Texture2D);
            shader->BindTexture("ssaoNoise", m_NoiseMap->GetID(), 13, SamplerType::Texture2D);
            shader->BindTexture("mainImage", graphicsContext->mainImage, 12, SamplerType::Texture2D);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            utils::RenderQuad(shader, QuadRenderSpecification::Screen);

            shader->Unbind();
        }

        virtual void OnResize(uint32_t w, uint32_t h) override
        {
            m_Framebuffer->OnResize(w, h);
            glViewport(0, 0, w, h);
        }

        virtual uint32_t GetFramebufferImage() override { return m_Framebuffer->GetColorAttachmentID(0); }

    private:
        std::shared_ptr<Texture2D> m_NoiseMap;
        Noise                      noise;
    };
}  // namespace suplex