#pragma once
#include <cstddef>
#include <glm/fwd.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <type_traits>
#include <vector>
#include "Layer/Layer.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace suplex {
    class Application {
    public:
        Application(uint32_t viewportWidth, uint32_t viewportHeight);
        ~Application();

        void Initialize();
        void Cleanup();

        // Render loop
        void Run();

        void OnUpdate(float ts);

        void OnResize(uint32_t w, uint32_t h)
        {
            m_ViewportWidth  = w;
            m_ViewportHeight = h;
        }

        void PushLayer(const std::shared_ptr<Layer> layer) { m_LayerStack.push_back(layer); }

        template <typename T>
        void PushLayer()
        {
            if (!std::is_base_of_v<Layer, T>) {
                spdlog::error("Not Implement from Layer");
                return;
            }
            spdlog::debug("Push Layer");
            m_LayerStack.emplace_back(std::make_shared<T>());
            spdlog::debug("layer stack size = {}", m_LayerStack.size());
        }

    private:
        void FramebufferPass();
        void InitImGui();

    private:
        bool     m_Running       = true;
        uint32_t m_ViewportWidth = 1920, m_ViewportHeight = 1080;

        std::vector<std::shared_ptr<Layer>> m_LayerStack;

        float m_lastTime = 1.0f;
    };

    std::shared_ptr<Application> CreateApplication(int argc, char** argv);
}  // namespace suplex