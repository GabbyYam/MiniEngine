#include "Application.hpp"
#include "EditorLayer.hpp"
#include "Render/Geometry/Model.hpp"
#include "EditorLayer.hpp"
#include <memory>
#include <spdlog/common.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <iostream>

#include "EntryPoint.hpp"
#include "Application.hpp"
#include "Render/RenderPass/ForwardPass.hpp"
#include "Render/Texture/Texture2D.hpp"

#include <glm/glm.hpp>
#include <Render/Camera/Camera.hpp>

std::shared_ptr<suplex::Application> suplex::CreateApplication(int argc, char** argv)
{
    // suplex::ApplicationSpecification spec;
    // spec.Name   = "Soft Renderer";
#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif

    int Width  = 1920 * 0.9;
    int Height = 1080 * 0.9;

    auto app = std::make_shared<suplex::Application>(Width, Height);
    app->PushLayer<EditorLayer>();
    return app;
}