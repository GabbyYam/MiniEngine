#include "Renderer.hpp"
#include "Camera/Camera.hpp"
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
#include <gl/gl.h>
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

glm::vec3 lightPositions[] = {
    glm::vec3(-10.0f, 10.0f, 10.0f),
    glm::vec3(10.0f, 10.0f, 10.0f),
    glm::vec3(-10.0f, -10.0f, 10.0f),
    glm::vec3(10.0f, -10.0f, 10.0f),
};

glm::vec3 lightColors[] = {
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f),
    glm::vec3(300.0f, 300.0f, 300.0f),
};

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
        m_Framebuffer.OnResize(m_ViewportWidth, m_ViewportHeight);
        m_Depthbuffer.OnResize(2048, 2048);

        m_Config            = std::make_shared<GraphicsConfig>();
        m_PrecomputeContext = std::make_shared<PrecomputeContext>();

        // Bake Environment Map
        BakeEnvironmentLight();

        // Bind Render Pass
        BindRenderPass();

        m_Bloom            = std::make_shared<Bloom>(1920, 1080, 5);
        m_BloomMergeShader = std::make_shared<Shader>("quad.vert", "postprocess.frag");
        // m_BloomMergeShader = std::make_shared<Shader>("quad.vert", "raymarching.frag");
        // debugShader        = std::make_shared<suplex::Shader>("quad.vert", "quad.frag");

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

    void Renderer::Render(const std::shared_ptr<Camera> camera)
    {
        m_ActiveCamera = camera;
        Walnut::Timer timer;

        glViewport(0, 0, 2048, 2048);
        glCullFace(GL_FRONT);
        m_DepthPass->Render(camera, m_Scene, m_Config, nullptr);

        m_Framebuffer.Bind();
        glCullFace(GL_BACK);
        auto& shaders = m_ForwardPass->GetShaders();
        for (auto& shader : shaders) {
            // // Bind Depth buffer to forward sampler
            shader->Bind();

            shader->SetInt("useEnvMap", m_Config->lightSetting.useEnvMap);

            shader->BindTexture("DepthMap", m_Depthbuffer.GetTextureID(), 15, SamplerType::Texture2D);

            shader->BindTexture("IrradianceMap", m_PrecomputeContext->IrradianceMap.GetID(), 14, SamplerType::CubeMap);

            shader->BindTexture("PrefilterMap", m_PrecomputeContext->PrefilterMap.GetID(), 13, SamplerType::CubeMap);

            shader->BindTexture("BRDF_LUT", m_PrecomputeContext->BRDF_LUT.GetID(), 12, SamplerType::Texture2D);

            // Light position
            for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i) {
                glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
                newPos           = lightPositions[i];
                shader->SetFloat3("lightPositions[" + std::to_string(i) + "]", glm::value_ptr(newPos));
                shader->SetFloat3("lightColors[" + std::to_string(i) + "]", glm::value_ptr(lightColors[i]));
            }

            shader->Unbind();
        }
        m_ForwardPass->Render(camera, m_Scene, m_Config, m_PrecomputeContext);

        if (m_Config->lightSetting.useEnvMap) {
            auto cubemapShader = m_EnvMapPass->GetShaders()[0];
            cubemapShader->Bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrecomputeContext->EnvironmentMap.GetID());
            glUniform1i(glGetUniformLocation(cubemapShader->GetID(), "EnvironmentMap"), 0);
            cubemapShader->Unbind();

            m_EnvMapPass->Render(camera, m_Scene, m_Config, m_PrecomputeContext);
        }

        if (m_Config->postprocessSetting.enablePostprocess) {
            m_Bloom->Render(m_Framebuffer.GetTextureID1(), m_Config->postprocessSetting.bloomFilterRadius);

            m_BloomMergeShader->Bind();

            m_BloomMergeShader->SetInt("tonemappingType", (int)m_Config->postprocessSetting.tonemappingType);
            m_BloomMergeShader->SetFloat("exposure", &m_Config->postprocessSetting.exposure);

            m_BloomMergeShader->SetInt("enableBloom", m_Config->postprocessSetting.enableBloom);
            m_BloomMergeShader->SetFloat("bloomIntensity", &m_Config->postprocessSetting.bloomIntensity);

            m_BloomMergeShader->SetInt("enableFXAA", m_Config->postprocessSetting.enableFXAA);

            m_BloomMergeShader->BindTexture("scene", m_Framebuffer.GetTextureID0(), 0, SamplerType::Texture2D);
            m_BloomMergeShader->BindTexture("bloomBlur", m_Bloom->GetResultID(), 1, SamplerType::Texture2D);

            // Render result in screen space quad
            glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer.GetID());
            glViewport(0, 0, m_ViewportWidth, m_ViewportHeight);
            utils::RenderQuad(m_BloomMergeShader);

            m_BloomMergeShader->Unbind();
        }

        m_LastRenderTime = timer.ElapsedMillis();
    }

    void Renderer::OnUIRender() { m_UIRenderPass->Render(m_ActiveCamera, m_Scene, m_Config, m_PrecomputeContext); }

    void Renderer::OnUpdate(float ts)
    {
        // Vsync
        glfwMakeContextCurrent(g_WindowHandle);
        glfwSwapInterval((int)m_Config->vsync);

        // Polygon Mode
        switch (m_Config->polygonMode) {
            case PolygonMode::Shaded: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
            case PolygonMode::WireFrame: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
            default: break;
        }
    }

    void Renderer::OnResize(uint32_t w, uint32_t h)
    {
        if (m_ViewportWidth == w && m_ViewportHeight == h) return;

        m_ViewportWidth = w, m_ViewportHeight = h;
        m_Framebuffer.OnResize(w, h);
    }

    void Renderer::BakeEnvironmentLight()
    {
        // Allocate resource for GPU
        glDisable(GL_CULL_FACE);
        float resolution = m_Config->environmentMapResolution;
        m_PrecomputeContext->HDR_EnvironmentTexture.LoadData("H:/GameDev Asset/Textures/EnvironmentMap/newport_loft.hdr",
                                                             ImageFormat::RGBA32F);

        m_PrecomputeContext->EnvironmentMap.Allocate();
        m_PrecomputeContext->EnvironmentMap.AllocateCubeMap(resolution);

        m_PrecomputeContext->framebuffer.OnResize(resolution, resolution);

        m_PrecomputeContext->IrradianceMap.Allocate();
        m_PrecomputeContext->IrradianceMap.AllocateCubeMap(32);

        m_PrecomputeContext->PrefilterMap.Allocate();
        m_PrecomputeContext->PrefilterMap.AllocateMipCubeMap(128);

        m_PrecomputeContext->BRDF_LUT.Allocate(512, 512);

        // Precompute Pass
        m_PrecomputePass = std::make_shared<PrecomputePass>();
        m_PrecomputePass->BindFramebuffer(m_PrecomputeContext->framebuffer.GetID());
        m_PrecomputePass->BindRenderbuffer(m_PrecomputeContext->framebuffer.GetRenderbufferID());

        m_PrecomputePass->PushShader(std::make_shared<Shader>("equirectangularMap.vert", "equirectangularMap.frag"));
        m_PrecomputePass->PushShader(std::make_shared<Shader>("equirectangularMap.vert", "diffuse_integration.frag"));
        m_PrecomputePass->PushShader(std::make_shared<Shader>("prefilter.vert", "prefilter.frag"));
        m_PrecomputePass->PushShader(std::make_shared<Shader>("brdf_integration.vert", "brdf_integration.frag"));

        m_PrecomputePass->Render(m_ActiveCamera, m_Scene, m_Config, m_PrecomputeContext);
        glEnable(GL_CULL_FACE);
    }

    void Renderer::BindRenderPass()
    {
        // Depth Pass
        m_DepthPass = m_PassQueue.emplace_back(std::make_shared<DepthRenderPass>());
        m_DepthPass->PushShader(std::make_shared<Shader>("depth.vert", "depth.frag"));
        m_DepthPass->BindFramebuffer(m_Depthbuffer.GetID());

        // Forward Pass
        m_Framebuffer.Bind();
        m_ForwardPass = m_PassQueue.emplace_back(std::make_shared<ForwardRenderPass>());
        m_ForwardPass->PushShader(std::make_shared<Shader>("common.vert", "pbr.frag"));
        m_ForwardPass->PushShader(std::make_shared<Shader>("phong.vert", "phong.frag"));
        m_ForwardPass->PushShader(std::make_shared<Shader>("toon.vert", "toon.frag"));
        m_ForwardPass->PushShader(std::make_shared<Shader>("common.vert", "light.frag"));

        m_ForwardPass->BindFramebuffer(m_Framebuffer.GetID());

        // EnvironmentMap Pass
        m_Framebuffer.Bind();
        m_EnvMapPass = m_PassQueue.emplace_back(std::make_shared<CubeMapPass>());
        m_EnvMapPass->PushShader(std::make_shared<Shader>("cubemap.vert", "cubemap.frag"));
        m_EnvMapPass->BindFramebuffer(m_Framebuffer.GetID());

        // Postprocessing Pass
        // m_Framebuffer.Bind();
        // m_PostprocessPass = m_PassQueue.emplace_back(std::make_shared<PostprocessPass>());
        // m_PostprocessPass->BindFramebuffer(m_Framebuffer.GetID());

        // UI RenderPass
        m_UIRenderPass = std::make_shared<suplex::ImGuiRenderPass>();
    }

}  // namespace suplex