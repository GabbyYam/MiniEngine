#include "Renderer.hpp"
#include "Camera/Camera.hpp"
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include "Render/Buffer/Depthbuffer.hpp"
#include "Render/Buffer/Framebuffer.hpp"
#include "Render/Config/Config.hpp"
#include "Render/Geometry/Shape/Shape.hpp"
#include "Render/Postprocess/Bloom.hpp"
#include "Render/RenderPass/CubeMapPass.hpp"
#include "Render/RenderPass/ForwardPass.hpp"
#include "Render/RenderPass/PostprocessPass.hpp"
#include "Render/RenderPass/PrecomputePass.hpp"
#include "Render/RenderPass/RenderPass.hpp"
#include "Render/RenderPass/CubeMapPass.hpp"
#include "Render/Renderer.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include "Render/Texture/Texture2D.hpp"
#include "Shader/Shader.hpp"
#include "Texture/CubeMap.hpp"
#include "Texture/Texture.hpp"
#include "Time/Timer.h"

extern GLFWwindow* g_WindowHandle;

suplex::Texture2D solidWhite;

namespace suplex {
    Renderer::Renderer()
    {
        // Buffer setting
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        // m_Framebuffer = Framebuffer(m_ViewportWidth, m_ViewportHeight);
        m_Framebuffer->OnResize(m_ViewportWidth, m_ViewportHeight);

        m_Context = std::make_shared<GraphicsContext>();
        // m_Context->config   = std::make_shared<GraphicsConfig>();
        m_PrecomputeContext = std::make_shared<PrecomputeContext>();

        // Bake Environment Map
        BakeEnvironmentLight();

        // Bind Render Pass
        BindRenderPass();

        m_Bloom            = std::make_shared<Bloom>(1920, 1080, 5);
        m_BloomMergeShader = std::make_shared<Shader>("quad.vert", "postprocess.frag");

        solidWhite.Allocate(16, 16);
        solidWhite.SolidColor();

        m_Scene = std::make_shared<Scene>();
    }

    Renderer::~Renderer()
    {
        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------
        // for (auto& model : m_Scene) {
        //     for (auto& mesh : model->GetMeshes()) { mesh.Unbind(); }
        // }
    }

    void Renderer::Render(const std::shared_ptr<Camera> camera, RenderType renderType)
    {
        m_ActiveCamera = camera;
        Walnut::Timer timer;

        m_DepthPass->Render(m_Context->config->lightSetting.cameraLS, m_Scene, m_Context, m_PrecomputeContext);
        m_DepthPass2->Render(camera, m_Scene, m_Context, m_PrecomputeContext);
        m_ForwardPass->Render(camera, m_Scene, m_Context, m_PrecomputeContext);

        m_LastRenderTime = timer.ElapsedMillis();
    }

    void Renderer::PostProcess(const std::shared_ptr<Camera> camera, RenderType renderType)
    {
        auto config = m_Context->config;
        if (config->lightSetting.useEnvMap) {
            auto cubemapShader = m_EnvMapPass->GetShaders()[0];
            cubemapShader->Bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrecomputeContext->EnvironmentMap.GetID());
            glUniform1i(glGetUniformLocation(cubemapShader->GetID(), "EnvironmentMap"), 0);
            cubemapShader->Unbind();

            m_EnvMapPass->Render(camera, m_Scene, m_Context, m_PrecomputeContext);
        }

        if (config->postprocessSetting.enablePostprocess) {
            m_Bloom->Render(m_Framebuffer->GetTextureID1(), config->postprocessSetting.bloomFilterRadius);

            m_BloomMergeShader->Bind();

            m_BloomMergeShader->SetFloat3("viewPos", glm::value_ptr(camera->GetPosition()));
            m_BloomMergeShader->SetFloat("nearClip", camera->GetNearClip());
            m_BloomMergeShader->SetFloat("farClip", camera->GetFarClip());

            m_BloomMergeShader->SetInt("tonemappingType", (int)config->postprocessSetting.tonemappingType);
            m_BloomMergeShader->SetFloat("exposure", &config->postprocessSetting.exposure);

            m_BloomMergeShader->SetInt("enableBloom", config->postprocessSetting.enableBloom);
            m_BloomMergeShader->SetFloat("bloomIntensity", &config->postprocessSetting.bloomIntensity);

            m_BloomMergeShader->SetInt("enableFXAA", config->postprocessSetting.enableFXAA);
            m_BloomMergeShader->SetInt("fogType", (int)config->postprocessSetting.fogType);
            m_BloomMergeShader->SetFloat("fogDensity", config->postprocessSetting.fogDensity);

            m_BloomMergeShader->BindTexture("scene", m_Framebuffer->GetTextureID0(), 0, SamplerType::Texture2D);
            m_BloomMergeShader->BindTexture("bloomBlur", m_Bloom->GetResultID(), 1, SamplerType::Texture2D);
            m_BloomMergeShader->BindTexture("DepthMap", m_DepthPass2->GetDepthMapID(), 2, SamplerType::Texture2D);

            // Render result in screen space quad
            glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer->GetID());
            glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);

            utils::RenderQuad(m_BloomMergeShader, QuadRenderSpecification::Screen);

            m_BloomMergeShader->Unbind();
        }
    }

    void Renderer::OnUIRender() { m_UIRenderPass->Render(m_ActiveCamera, m_Scene, m_Context, m_PrecomputeContext); }

    void Renderer::OnUpdate(float ts)
    {
        // Vsync
        auto config = m_Context->config;
        glfwMakeContextCurrent(g_WindowHandle);
        glfwSwapInterval((int)config->vsync);

        // Polygon Mode
        switch (config->polygonMode) {
            case PolygonMode::Shaded: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
            case PolygonMode::WireFrame: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
            default: break;
        }
    }

    void Renderer::OnResize(uint32_t w, uint32_t h)
    {
        if (m_ViewportWidth == w && m_ViewportHeight == h) return;

        m_ViewportWidth = w, m_ViewportHeight = h;
        m_Framebuffer->OnResize(w, h);
    }

    void Renderer::BakeEnvironmentLight()
    {
        // Allocate resource for GPU
        auto config = m_Context->config;
        glDisable(GL_CULL_FACE);
        float resolution = config->environmentMapResolution;
        m_PrecomputeContext->HDR_EnvironmentTexture.LoadData("H:/GameDev Asset/Textures/EnvironmentMap/newport_loft.hdr",
                                                             ImageFormat::RGBA32F);
        m_PrecomputeContext->EnvironmentMap.Allocate();
        m_PrecomputeContext->EnvironmentMap.AllocateCubeMap(resolution);

        m_PrecomputeContext->precomputeFrambuffer->OnResize(resolution, resolution);

        m_PrecomputeContext->IrradianceMap.Allocate();
        m_PrecomputeContext->IrradianceMap.AllocateCubeMap(32);

        m_PrecomputeContext->PrefilterMap.Allocate();
        m_PrecomputeContext->PrefilterMap.AllocateMipCubeMap(128);

        m_PrecomputeContext->BRDF_LUT.Allocate(512, 512);

        // Precompute Pass
        m_PrecomputePass = std::make_shared<PrecomputePass>();
        m_PrecomputePass->BindFramebuffer(m_PrecomputeContext->precomputeFrambuffer);

        m_PrecomputePass->PushShader(std::make_shared<Shader>("equirectangularMap.vert", "equirectangularMap.frag"));
        m_PrecomputePass->PushShader(std::make_shared<Shader>("equirectangularMap.vert", "diffuse_integration.frag"));
        m_PrecomputePass->PushShader(std::make_shared<Shader>("prefilter.vert", "prefilter.frag"));
        m_PrecomputePass->PushShader(std::make_shared<Shader>("brdf_integration.vert", "brdf_integration.frag"));

        m_PrecomputePass->Render(m_ActiveCamera, m_Scene, m_Context, m_PrecomputeContext);
        glEnable(GL_CULL_FACE);
    }

    void Renderer::BindRenderPass()
    {
        // Depth Pass
        m_DepthPass  = m_PassQueue.emplace_back(std::make_shared<DepthRenderPass>());
        m_DepthPass2 = m_PassQueue.emplace_back(std::make_shared<DepthRenderPass>());

        // Forward Pass
        m_Framebuffer->Bind();
        m_ForwardPass = m_PassQueue.emplace_back(std::make_shared<ForwardRenderPass>());

        m_ForwardPass->BindFramebuffer(m_Framebuffer);
        m_ForwardPass->BindDepthbuffer(m_DepthPass->GetDepthbuffer());

        // EnvironmentMap Pass
        m_Framebuffer->Bind();
        m_EnvMapPass = m_PassQueue.emplace_back(std::make_shared<CubeMapPass>());
        m_EnvMapPass->PushShader(std::make_shared<Shader>("cubemap.vert", "cubemap.frag"));
        m_EnvMapPass->BindFramebuffer(m_Framebuffer);

        // Postprocessing Pass
        // m_Framebuffer->Bind();
        // m_PostprocessPass = m_PassQueue.emplace_back(std::make_shared<PostprocessPass>());
        // m_PostprocessPass->BindFramebuffer(m_Framebuffer->GetID());

        // UI RenderPass
        m_UIRenderPass = std::make_shared<suplex::ImGuiRenderPass>();
    }

}  // namespace suplex