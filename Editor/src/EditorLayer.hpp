#pragma once

#include "Panel/ContentBrowserPanel.hpp"
#include "Panel/Panel.hpp"
#include "Panel/SceneHirarchyPanel.hpp"
#include "Platform/Windows/FileDialogs.hpp"
#include "Render/Postprocess/PostProcess.hpp"
#include "Render/Renderer.hpp"
#include "Render/Camera/Camera.hpp"
#include "GLFW/glfw3.h"
#include "Render/Geometry/Model.hpp"
#include "Layer/Layer.hpp"
#include "Render/RenderPass/RenderPass.hpp"
#include "Render/Renderer.hpp"
#include "Render/Texture/Texture.hpp"
#include "Scene/Component/Component.hpp"
#include "Scene/Scene.hpp"
#include "Scene/SceneSerilizer.hpp"
#include "imgui.h"
#include <ImGuizmo.h>
#include <array>
#include <cstddef>
#include <filesystem>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "IconsFontAwesome6.h"
#include "imgui_internal.h"
#include "InputSystem/Input.h"
#include "InputSystem/KeyCodes.h"
#include "Widget/CommonWidget.hpp"
#include <Scene/Scene.hpp>
#include <Scene/Entity/Entity.hpp>
#include <winscard.h>

const static std::array<std::string, 2> PolygonNames{"Shaded", "WireFrame"};
const static std::array<std::string, 3> ToneMappingNames{"Logarithmic", "ACES", "None"};
const static std::array<std::string, 2> OperationModes{"Local", "World"};

static float randomRadians = rand() % 100;

namespace suplex {

    class EditorLayer : public Layer {
    public:
        EditorLayer() {}

        ~EditorLayer() {}

        virtual void OnAttach() override
        {
            // Renderer
            m_Renderer = std::make_shared<Renderer>();

            // Scene
            // std::string filename      = "H:/GameDev Asset/Models/Vroid_JK.fbx";
            // std::string sceneName     = "H:/GameDev Asset/Scenes/Sponza/sponza.obj";
            // std::string stagefileName = "H:/GameDev Asset/Scenes/Simple Stage/park.fbx";
            // std::string sphereName    = "H:/GameDev Asset/Models/sphere.obj";

            // auto vroidJK     = std::make_shared<Model>(filename);
            // auto simpleStage = std::make_shared<Model>(stagefileName);
            // auto sphere      = std::make_shared<Model>(sphereName);

            // Camera
            m_Camera = std::make_shared<Camera>(45.0f, 0.01f, 10000.f);

            // Bind Context for Panels
            m_Panels.emplace_back(std::make_shared<SceneHirarchyPanel>());
            m_Panels.emplace_back(std::make_shared<ContentBrowserPanel>());

            RuntimeContext context{
                .renderer = m_Renderer,
                .camera   = m_Camera,
            };

            // auto e1 = m_Renderer->GetScene()->CreateEntity("Vroid JK");
            // e1.AddComponent<TransformComponent>(glm::vec3{0.f, 1.2f, 0.f}, glm::vec3{-90.0f, 0.0f, 0.0f});
            // e1.AddComponent<MeshRendererComponent>(vroidJK);

            // auto e2 = m_Renderer->GetScene()->CreateEntity("Simple Stage");
            // e2.AddComponent<TransformComponent>(glm::vec3{0.f, 1.f, -3.f}, glm::vec3{-90.0f, 0.0f, 0.0f});
            // e2.AddComponent<MeshRendererComponent>(simpleStage);

            // auto e3 = m_Renderer->GetScene()->CreateEntity("Sphere");
            // e3.AddComponent<TransformComponent>(glm::vec3{0.f, 5.f, 5.f});
            // e3.AddComponent<MeshRendererComponent>(sphere);

            m_Context = std::make_shared<RuntimeContext>(context);
            for (auto& panel : m_Panels) panel->SetContext(m_Context);

            m_SceneSerilizer = std::make_shared<SceneSerializer>(m_Renderer->GetScene());
        }

        virtual void OnUpdate(float ts) override
        {
            OnEvent();
            m_Camera->OnUpdate(ts);

            if (m_Play) m_Renderer->OnUpdate(ts);

            m_Renderer->Render(m_Camera);

            float currentTime = glfwGetTime();
            m_DeltaTime       = currentTime - m_LastTime;
            m_LastTime        = currentTime;
        }

        virtual void OnUIRender() override
        {
            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

            // We are using the ImGuiWindowFlags_NoDocking flag to make the
            // parent window not dockable into, because it would be
            // confusing to have two docking targets within each others.
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;

            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |=
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::Begin("DockSpace Demo", nullptr, window_flags);
            ImGui::PopStyleVar();

            ImGui::PopStyleVar(2);

            // Submit the DockSpace
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
                ImGuiID dockspace_id = ImGui::GetID("AppDockspace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
            }

            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Open")) {
                        std::string filename = FileDialogs::OpenFile("Suplex Engine Scene (*.suplex; *.txt)\0");
                        if (!filename.empty()) {
                            info("Open File {}", filename);
                            OpenScene(std::filesystem::path(filename.data()));
                        }
                    }

                    if (ImGui::MenuItem("Save")) {
                        std::string filename = FileDialogs::SaveFile("Suplex Engine Scene (*.suplex; *.txt)\0");
                        if (!filename.empty()) {
                            info("Scene Save to {}", filename);
                            m_SceneSerilizer->Serialize(filename);
                        }
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            auto& config = m_Renderer->GetGraphicsConfig();
            {
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0, 0.0});

                // ImGui::Begin(ICON_FA_CUBE "  Viewport");
                ImGui::Begin("Scene");
                auto windowSize = ImGui::GetContentRegionAvail();
                this->OnResize(windowSize.x, windowSize.y);

                auto& camera = m_Context->camera;
                assert(camera != nullptr);
                widget::EditTransform(glm::value_ptr(camera->GetView()), glm::value_ptr(camera->GetProjection()), m_ActiveOperation,
                                      m_ActiveMode);

                ImGui::Image((void*)(intptr_t)m_Renderer->FramebufferImageID(), ImGui::GetContentRegionAvail(), {0.0f, 1.0f},
                             {1.0f, 0.0f});

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                        const wchar_t* path = (const wchar_t*)payload->Data;
                        OpenScene(path);
                    }
                    ImGui::EndDragDropTarget();
                }

                auto activeEntity = m_Renderer->GetScene()->GetActiveEntity();
                if (activeEntity) {
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist();
                    float w = (float)ImGui::GetWindowWidth(), h = (float)ImGui::GetWindowHeight();

                    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, w, h);

                    auto& transformComponent = activeEntity.GetComponent<TransformComponent>();
                    auto  transform          = transformComponent.GetTransform();

                    ImGuizmo::Manipulate(glm::value_ptr(camera->GetView()), glm::value_ptr(camera->GetProjection()), m_ActiveOperation,
                                         m_ActiveMode, glm::value_ptr(transform));

                    if (ImGuizmo::IsUsing()) { transformComponent.SetTransform(transform); }
                }

                ImGui::End();
                ImGui::PopStyleVar(1);
            }

            {
                // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.0, 0.0});
                // ImGui::Begin("Depth Buffer (Light Space)");

                // ImGui::Image((void*)(intptr_t)m_Renderer->DepthMapID(), ImGui::GetContentRegionAvail(), {0.0f, 1.0f}, {1.0f, 0.0f});
                // ImGui::End();
                // ImGui::PopStyleVar(1);
            }

            {
                // ImGui::Begin(ICON_FA_TOOLS "  Setting");
                ImGui::Begin("Setting");

                ImGui::Text("Frame time = %.3f ms", m_Renderer->LastFrameRenderTime());
                ImGui::Text("Frame rate = %.3f fps", 1000.0 / m_Renderer->LastFrameRenderTime());
                ImGui::Checkbox("Play", &m_Play);
                ImGui::Checkbox("Show Demo Window", &m_ShowDemoWindow);
                ImGui::Checkbox("Vsync", &config->vsync);

                // Set Polygon Mode
                {
                    widget::ItemLabel("Polygon Mode", widget::Left);
                    int index = (int)config->polygonMode;

                    std::string previewName = PolygonNames[index];
                    if (ImGui::BeginCombo("Polygon Mode", previewName.data(), 0)) {
                        for (int i = 0; i < PolygonNames.size(); i++) {
                            const bool is_selected = (index == i);
                            if (ImGui::Selectable(PolygonNames[i].data(), is_selected)) config->polygonMode = (PolygonMode)i;
                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                }

                {
                    widget::ItemLabel("Operation Mode", widget::Left);

                    int         index       = (int)m_ActiveMode;
                    std::string previewName = OperationModes[index];
                    if (ImGui::BeginCombo("##Operation Mode", previewName.data(), 0)) {
                        for (int i = 0; i < OperationModes.size(); i++) {
                            const bool is_selected = (index == i);
                            if (ImGui::Selectable(OperationModes[i].data(), is_selected)) m_ActiveMode = (ImGuizmo::MODE)i;
                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }
                }

                // ImGui::Text("Camera: x = %.1f y = %.1f z = %.1f", m_Camera->GetPosition()[0], m_Camera->GetPosition()[1],
                //             m_Camera->GetPosition()[2]);
                // ImGui::SliderFloat("Camera Move Speed", &m_CameraSpeed, 1.0f, 20.0f);

                ImGui::End();
            }

            {
                // ImGui::Begin(ICON_FA_LIGHTBULB "  Light Setting");
                ImGui::Begin("Light Setting");
                // Direcitonal Light
                auto& cameraLS   = config->lightSetting.cameraLS;
                bool  viewChange = false;
                ImGui::Text("Directional Light");
                viewChange |= ImGui::SliderFloat3("Light Direciton", &cameraLS->GetForward()[0], -3.14f * 2, 3.14f * 2);
                ImGui::SliderFloat("Light Intensity", &config->lightSetting.lightIntensity, 1.0f, 8.0f);
                ImGui::ColorEdit3("Light Color", &config->lightSetting.lightColor[0]);

                bool distanceChange = false;
                distanceChange |= ImGui::InputFloat("Light Camera Near", &cameraLS->GetNearClip());
                distanceChange |= ImGui::InputFloat("Light Camera Far", &cameraLS->GetFarClip());
                if (distanceChange) cameraLS->RecalculateOrthoProjection();

                // Rotate Directional Light
                float        radius = 10;
                static float time   = 0;
                if (m_Play) time += m_DeltaTime * 0.2;

                cameraLS->GetPosition() = vec3(radius * sin(time), cameraLS->GetPosition().y, radius * cos(time));
                cameraLS->GetForward()  = vec3(0.0f) - cameraLS->GetPosition();
                cameraLS->LookAtWorldCenter();

                // Shadow
                ImGui::Text("Shadow");
                ImGui::Checkbox("Cast Shadow", &config->lightSetting.castShadow);

                ImGui::Text("Ambient");
                ImGui::Checkbox("Use EnvMap", &config->lightSetting.useEnvMap);

                ImGui::End();
            }

            {
                ImGui::Begin("Postprocess");
                ImGui::Checkbox("Enable Postprocess", &config->postprocessSetting.enablePostprocess);

                if (config->postprocessSetting.enablePostprocess) {
                    ImGui::Text("Tone Mapping");
                    {
                        // Tone mapping type
                        int         index       = (int)config->postprocessSetting.tonemappingType;
                        std::string previewName = ToneMappingNames[index];
                        if (ImGui::BeginCombo("Tone Mapping", previewName.data(), 0)) {
                            for (int i = 0; i < ToneMappingNames.size(); i++) {
                                const bool is_selected = (index == i);
                                if (ImGui::Selectable(ToneMappingNames[i].data(), is_selected))
                                    config->postprocessSetting.tonemappingType = (ToneMappingType)i;
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected) ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        if (config->postprocessSetting.tonemappingType == ToneMappingType::Logarithmic)
                            ImGui::SliderFloat("Exposure", &config->postprocessSetting.exposure, 0.1, 5.0);
                    }

                    ImGui::Text("Bloom");
                    ImGui::Checkbox("Enable Global Bloom", &config->postprocessSetting.enableBloom);
                    if (config->postprocessSetting.enableBloom) {
                        ImGui::SliderFloat("Bloom Threshold", &config->postprocessSetting.bloomThreshold, 0.5, 10.0);
                        ImGui::SliderFloat("Bloom Intensity", &config->postprocessSetting.bloomIntensity, 0.00, 0.8);
                        ImGui::SliderFloat("Bloom Filter Radius", &config->postprocessSetting.bloomFilterRadius, 0.001, 0.03);
                    }

                    ImGui::Text("Anti-aliasing");
                    ImGui::Checkbox("FXAA", &config->postprocessSetting.enableFXAA);
                }

                ImGui::End();
            }

            {
                ImGui::Begin("PBR Material");
                ImGui::ColorEdit3("Albedo", glm::value_ptr(config->pbrSetting.baseColor));
                ImGui::SliderFloat("Metalic", &config->pbrSetting.metallic, 0.0, 1.0);
                ImGui::SliderFloat("Roughness", &config->pbrSetting.roughness, 0.1, 1.0);
                ImGui::SliderFloat("AO", &config->pbrSetting.ao, 0.0, 1.0);
                ImGui::End();
            }

            ImGui::End();

            // Render Panel
            for (auto panel : m_Panels) panel->OnUIRender();

            if (m_ShowDemoWindow) ImGui::ShowDemoWindow();

            m_Renderer->OnUIRender();
        }

        virtual void OnResize(uint32_t w, uint32_t h) override
        {
            if (w == m_ViewportWidth && h == m_ViewportHeight) return;
            m_ViewportWidth  = w;
            m_ViewportHeight = h;

            m_Renderer->OnResize(w, h);
            m_Camera->OnResize(w, h);
        }

        void OnEvent()
        {
            if (Input::IsMouseButtonDown(MouseButton::Right)) return;
            if (Input::IsKeyDown(KeyCode::W)) m_ActiveOperation = ImGuizmo::OPERATION::TRANSLATE;
            if (Input::IsKeyDown(KeyCode::E)) m_ActiveOperation = ImGuizmo::OPERATION::SCALE;
            if (Input::IsKeyDown(KeyCode::R)) m_ActiveOperation = ImGuizmo::OPERATION::ROTATE;
            // if (Input::IsMouseButtonDown(MouseButton::Left) && ImGui::IsWindowHovered()) m_Context->activeEntity = -1;
        }

        void OpenScene();
        void OpenScene(std::filesystem::path const& path);

    private:
        std::shared_ptr<Renderer> m_Renderer      = nullptr;
        uint32_t                  m_ViewportWidth = 1920, m_ViewportHeight = 1080;

        std::shared_ptr<Camera> m_Camera = nullptr;

        bool m_ShowDemoWindow = false;
        bool m_Play           = true;

        std::vector<std::shared_ptr<Panel>> m_Panels;

        std::shared_ptr<SceneSerializer> m_SceneSerilizer;

        float m_LastTime  = 0.0f;
        float m_DeltaTime = 0.0f;

        std::shared_ptr<RuntimeContext> m_Context = nullptr;

        ImGuizmo::OPERATION m_ActiveOperation = ImGuizmo::OPERATION::TRANSLATE;
        ImGuizmo::MODE      m_ActiveMode      = ImGuizmo::MODE::LOCAL;
    };
}  // namespace suplex