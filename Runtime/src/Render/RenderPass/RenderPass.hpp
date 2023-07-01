#pragma once

#include "Render/Buffer/Depthbuffer.hpp"
#include "Render/Buffer/HdrFramebuffer.hpp"
#include "Render/Geometry/Mesh.hpp"
#include "Render/Geometry/Model.hpp"
#include "Render/Buffer/Framebuffer.hpp"
#include "Render/Config/Config.hpp"
#include "Render/Shader/Shader.hpp"
#include "Render/Camera/Camera.hpp"
#include "Scene/Component/Component.hpp"
#include "Scene/Scene.hpp"
#include "glm/ext.hpp"
#include <ImGuizmo.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <vector>

namespace suplex {

    struct PrecomputeContext
    {
        Texture2D                       HDR_EnvironmentTexture;
        CubeMap                         EnvironmentMap;
        CubeMap                         IrradianceMap;
        CubeMap                         PrefilterMap;
        Texture2D                       BRDF_LUT;
        std::shared_ptr<HdrFramebuffer> precomputeFrambuffer = std::make_shared<HdrFramebuffer>();
    };

    class RenderPass {
    public:
        virtual void Render(const std::shared_ptr<Camera>            camera,
                            const std::shared_ptr<Scene>             scene,
                            const std::shared_ptr<GraphicsContext>   graphicsContext,
                            const std::shared_ptr<PrecomputeContext> context)
        {
        }

        virtual void Render(uint32_t framebufferID) {}

        virtual void OnResize(uint32_t w, uint32_t h) {}

        virtual void OnEnable() { m_Running = true; }
        virtual void OnDisable() { m_Running = false; }

        void Awake(bool value) { m_Running = value; }

        void BindFramebuffer(const std::shared_ptr<HdrFramebuffer> framebuffer) { m_Framebuffer = framebuffer; }
        void UnbindFramebuffer() { m_Framebuffer = nullptr; }

        void BindDepthbuffer(const std::shared_ptr<Depthbuffer> depthbuffer) { m_Depthbuffer = depthbuffer; }

        auto& GetShaders() { return m_Shaders; }

        auto PushShader(const std::shared_ptr<Shader> shader) { return m_Shaders.emplace_back(shader); }

    protected:
        std::vector<std::shared_ptr<Shader>> m_Shaders;
        std::shared_ptr<HdrFramebuffer>      m_Framebuffer;
        std::shared_ptr<Depthbuffer>         m_Depthbuffer;

        bool m_Running = true;
    };

    class DepthRenderPass : public RenderPass {
        virtual void Render(const std::shared_ptr<Camera>            camera,
                            const std::shared_ptr<Scene>             scene,
                            const std::shared_ptr<GraphicsContext>   graphicsContext,
                            const std::shared_ptr<PrecomputeContext> context) override
        {
            // render to custom framebuffer
            // ------
            glBindFramebuffer(GL_FRAMEBUFFER, m_Depthbuffer->GetID());
            glClear(GL_DEPTH_BUFFER_BIT);
            auto  config      = graphicsContext->config;
            auto& lightCamera = config->lightSetting.cameraLS;
            auto  viewLS      = lightCamera->GetView();
            auto  projLS      = lightCamera->GetOrthoProjection();

            auto& shader = m_Shaders[0];
            shader->Bind();
            int viewIndex = glGetUniformLocation(shader->GetID(), "viewLS");
            glUniformMatrix4fv(viewIndex, 1, GL_FALSE, glm::value_ptr(viewLS));

            int projIndex = glGetUniformLocation(shader->GetID(), "projLS");
            glUniformMatrix4fv(projIndex, 1, GL_FALSE, glm::value_ptr(projLS));

            auto entities = scene->m_Registry.view<MeshRendererComponent>();
            for (auto& entity : entities) {
                auto transform = scene->m_Registry.get<TransformComponent>(entity);

                shader->SetMaterix4("model", glm::value_ptr(transform.GetTransform()));
                auto& meshRenderer = scene->m_Registry.get<MeshRendererComponent>(entity);
                for (auto& mesh : meshRenderer.m_Model->GetMeshes()) mesh.Render(shader);
            }

            shader->Unbind();

            // Return to default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    };

    class ImGuiRenderPass : public RenderPass {
    public:
        virtual void Render(const std::shared_ptr<Camera>            camera,
                            const std::shared_ptr<Scene>             scene,
                            const std::shared_ptr<GraphicsContext>   graphicsContext,
                            const std::shared_ptr<PrecomputeContext> context) override
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // glClearColor(.6f, 0.7f, 0.9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                auto backupWindow = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backupWindow);
            }

            // Return to default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

    private:
    };

}  // namespace suplex