#pragma once

#include "Render/Buffer/Framebuffer.hpp"
#include "Render/Buffer/HdrFramebuffer.hpp"
#include "Render/Postprocess/Noise.hpp"
#include "Render/RenderPass/RenderPass.hpp"
#include "Render/Texture/Texture2D.hpp"
#include <Render/Postprocess/Bloom.hpp>
#include <memory>
#include <stdint.h>
namespace suplex {
    class PostprocessPass : public RenderPass {
    public:
        PostprocessPass()
        {
            m_Bloom       = std::make_shared<Bloom>(1920, 1080, 5);
            m_MergeShader = std::make_shared<Shader>("quad.vert", "postprocess.frag");
            FramebufferSpecification spec;
            spec.Attachments = {
                {TextureFormat::RGBA, TextureFilter::Linear, TextureWrap::ClampToEdge},
                {TextureFormat::RGBA, TextureFilter::Linear, TextureWrap::ClampToEdge},
                {TextureFormat::Depth, TextureFilter::Linear, TextureWrap::ClampToBorder},
                {TextureFormat::RED_INTEGER, TextureFilter::Nearest, TextureWrap::ClampToEdge},
            };

            auto  ssaoNoise  = noise.SSAONoise();
            auto  ssaoKernel = noise.SSAOKernel();
            void* data       = ssaoNoise.data();

            m_NoiseMap = std::make_shared<Texture2D>(data);
            m_MergeShader->Bind();
            for (int i = 0; i < 64; ++i)
                m_MergeShader->SetFloat3(("samples[" + std::to_string(i) + "]"), glm::value_ptr(ssaoKernel[i]));

            // Framebuffer out();
            m_OutputFramebuffer = std::make_shared<Framebuffer>(spec);
        }

        virtual void Render(const std::shared_ptr<Camera>            camera,
                            const std::shared_ptr<Scene>             scene,
                            const std::shared_ptr<GraphicsContext>   graphicsContext,
                            const std::shared_ptr<PrecomputeContext> context) override
        {
            auto config = graphicsContext->config;

            if (config->postprocessSetting.enablePostprocess) {
                m_Bloom->Render(m_Framebuffer->GetColorAttachmentID(1), config->postprocessSetting.bloomFilterRadius);

                m_MergeShader->Bind();

                m_MergeShader->SetMaterix4("projection", glm::value_ptr(camera->GetProjection()));

                m_MergeShader->SetFloat3("viewPos", glm::value_ptr(camera->GetPosition()));
                m_MergeShader->SetFloat("nearClip", camera->GetNearClip());
                m_MergeShader->SetFloat("farClip", camera->GetFarClip());

                m_MergeShader->SetInt("tonemappingType", (int)config->postprocessSetting.tonemappingType);
                m_MergeShader->SetFloat("exposure", &config->postprocessSetting.exposure);

                m_MergeShader->SetInt("enableBloom", config->postprocessSetting.enableBloom);
                m_MergeShader->SetFloat("bloomIntensity", &config->postprocessSetting.bloomIntensity);

                m_MergeShader->SetInt("enableFXAA", config->postprocessSetting.enableFXAA);
                m_MergeShader->SetInt("fogType", (int)config->postprocessSetting.fogType);
                m_MergeShader->SetFloat("fogDensity", config->postprocessSetting.fogDensity);
                m_MergeShader->SetFloat("fogStart", config->postprocessSetting.fogStart);
                m_MergeShader->SetFloat("fogEnd", config->postprocessSetting.fogEnd);

                m_MergeShader->SetInt("enableDoF", config->postprocessSetting.enableDoF);

                m_MergeShader->BindTexture("scene", m_Framebuffer->GetColorAttachmentID(0), 0, SamplerType::Texture2D);
                m_MergeShader->BindTexture("bloomBlur", m_Bloom->GetResultID(), 1, SamplerType::Texture2D);

                m_MergeShader->SetInt("enableSSAO", config->postprocessSetting.enableSSAO);
                m_MergeShader->BindTexture("DepthMap", graphicsContext->depthMap, 2, SamplerType::Texture2D);
                m_MergeShader->BindTexture("gPosition", graphicsContext->gPosition, 3, SamplerType::Texture2D);
                m_MergeShader->BindTexture("gNormal", graphicsContext->gNormal, 4, SamplerType::Texture2D);
                m_MergeShader->BindTexture("ssaoNoise", m_NoiseMap->GetID(), 5, SamplerType::Texture2D);

                // Render result in screen space quad
                m_OutputFramebuffer->Bind();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                utils::RenderQuad(m_MergeShader, QuadRenderSpecification::Screen);

                m_MergeShader->Unbind();
            }
        }

        virtual void OnResize(uint32_t w, uint32_t h) override
        {
            m_OutputFramebuffer->OnResize(w, h);
            glViewport(0, 0, w, h);
        }

        virtual uint32_t GetFramebufferImage() override { return m_OutputFramebuffer->GetColorAttachmentID(0); }

    private:
        std::shared_ptr<Bloom>       m_Bloom       = nullptr;
        std::shared_ptr<Shader>      m_MergeShader = nullptr;
        std::shared_ptr<Framebuffer> m_OutputFramebuffer;
        std::shared_ptr<Texture2D>   m_NoiseMap;
        Noise                        noise;
    };
}  // namespace suplex