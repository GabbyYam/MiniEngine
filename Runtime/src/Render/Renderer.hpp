#pragma once

#include "Camera/Camera.hpp"
#include "Render/Buffer/Depthbuffer.hpp"
#include "Render/Buffer/Framebuffer.hpp"
#include "Render/Buffer/HdrFramebuffer.hpp"
#include "Render/Camera/Camera.hpp"
#include "Render/Geometry/Model.hpp"
#include "Render/Postprocess/Bloom.hpp"
#include "Render/RenderPass/RenderPass.hpp"
#include "RenderPass/RenderPass.hpp"
#include "Scene/Scene.hpp"
#include "Shader/Shader.hpp"
#include "Texture/CubeMap.hpp"
#include "imgui.h"
#include <cstddef>
#include <glm/fwd.hpp>
#include <memory>
#include <stdint.h>
#include <vector>

namespace suplex {

    class Renderer {
    public:
        Renderer();
        virtual ~Renderer();
        void OnUpdate(float ts);
        void Render(const std::shared_ptr<Camera> camera);
        void OnUIRender();
        void OnResize(uint32_t w, uint32_t h);

        void OnAwake(bool value)
        {
            for (auto& pass : m_PassQueue) pass->Awake(value);
        }

        // void LoadModel(const std::shared_ptr<Model> model) { m_Scene.emplace_back(model); }

        uint32_t FramebufferID() const { return m_Framebuffer.GetID(); }
        uint32_t FramebufferImageID() const { return m_Framebuffer.GetTextureID0(); }
        uint32_t DepthbufferID() const { return m_Depthbuffer.GetID(); }
        uint32_t DepthMapID() const { return m_Depthbuffer.GetTextureID(); }
        float    LastFrameRenderTime() const { return m_LastRenderTime; }

        auto& GetGraphicsConfig() { return m_Config; }
        auto& GetGameObjectList() { return m_Scene; }
        auto& GetActiveObject() { return m_ActiveObejct; }
        auto& GetShadersList() { return m_ForwardPass->GetShaders(); }
        auto& GetScene() { return m_Scene; }

    private:
        void OnBufferResize();
        void BindRenderPass();
        void BakeEnvironmentLight();

    private:
        uint32_t m_ViewportWidth = 1920, m_ViewportHeight = 1080;

        HdrFramebuffer m_Framebuffer;
        Depthbuffer    m_Depthbuffer;

        std::vector<std::shared_ptr<RenderPass>> m_PassQueue;
        std::shared_ptr<RenderPass>              m_UIRenderPass    = nullptr;
        std::shared_ptr<RenderPass>              m_ForwardPass     = nullptr;
        std::shared_ptr<RenderPass>              m_DepthPass       = nullptr;
        std::shared_ptr<RenderPass>              m_EnvMapPass      = nullptr;
        std::shared_ptr<RenderPass>              m_PrecomputePass  = nullptr;
        std::shared_ptr<RenderPass>              m_PostprocessPass = nullptr;
        // std::vector<std::shared_ptr<Model>>      m_Scene;
        std::shared_ptr<Scene> m_Scene;

        std::shared_ptr<Camera> m_ActiveCamera = nullptr;
        std::shared_ptr<Model>  m_ActiveObejct = nullptr;

        std::shared_ptr<Bloom>  m_Bloom            = nullptr;
        std::shared_ptr<Shader> m_BloomMergeShader = nullptr;

        std::shared_ptr<Camera> m_LightCamera = nullptr;

        float m_LastRenderTime = 1.0f;

        std::shared_ptr<GraphicsConfig>    m_Config            = nullptr;
        std::shared_ptr<PrecomputeContext> m_PrecomputeContext = nullptr;
    };
}  // namespace suplex