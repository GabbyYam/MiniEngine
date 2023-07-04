#pragma once

#include "Panel/ContentBrowserPanel.hpp"
#include "Panel/Panel.hpp"
#include "Panel/SceneHirarchyPanel.hpp"
#include "Platform/Windows/FileDialogs.hpp"
#include "Render/Config/Config.hpp"
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
#include <corecrt_math.h>
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

const static std::array<std::string, 2> PolygonNames{"Shaded", "WireFrame"};
const static std::array<std::string, 3> ToneMappingNames{"None", "Logarithmic", "ACES"};
const static std::array<std::string, 2> OperationModes{"Local", "World"};

static float randomRadians = rand() % 100;

namespace suplex {

    class EditorLayer : public Layer {
    public:
        EditorLayer() {}

        ~EditorLayer() {}

        virtual void OnAttach() override
        {
            m_Renderer = std::make_shared<Renderer>();
            m_Camera   = std::make_shared<Camera>(45.0f, 0.01f, 1000.f, ProjectionType::Perspective);

            // Bind Context for Panels
            m_SceneHirarchyPanel = std::make_shared<SceneHirarchyPanel>();
            m_Panels.push_back(m_SceneHirarchyPanel);
            m_Panels.emplace_back(std::make_shared<ContentBrowserPanel>());

            RuntimeContext context{
                .renderer = m_Renderer,
                .camera   = m_Camera,
            };

            m_Context = std::make_shared<RuntimeContext>(context);
            for (auto& panel : m_Panels)
                panel->SetContext(m_Context);

            m_SceneSerilizer = std::make_shared<SceneSerializer>(m_Renderer->GetScene());
        }

        virtual void OnUpdate(float ts) override
        {
            m_Camera->OnUpdate(ts);

            if (m_Play)
                m_Renderer->OnUpdate(ts);

            m_Renderer->GetGraphicsContext()->activeEntity = m_SceneHirarchyPanel->GetSelectedEntity();
            m_Renderer->Render(m_Camera);

            float currentTime = glfwGetTime();
            m_DeltaTime       = currentTime - m_LastTime;
            m_LastTime        = currentTime;

            auto [mx, my] = ImGui::GetMousePos();
            mx -= m_ViewportBounds[0].x;
            my -= m_ViewportBounds[0].y;

            glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
            my                     = viewportSize.y - my;
            int mouseX             = (int)mx;
            int mouseY             = (int)my;

            if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y) {
                int pixelData = m_Renderer->m_Framebuffer->ReadPixel(2, mouseX, mouseY);
                m_HoveredEntity =
                    (pixelData == -1 || pixelData > 9961) ? Entity() : Entity((entt::entity)pixelData, m_Renderer->GetScene().get());
            }

            OnEvent();

            m_Renderer->PostProcess(m_Camera);
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

            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                window_flags |= ImGuiWindowFlags_NoBackground;

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

                ImGui::Begin("Scene");

                auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
                auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
                auto viewportOffset    = ImGui::GetWindowPos();
                m_ViewportBounds[0]    = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
                m_ViewportBounds[1]    = {viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y};

                m_ViewportHovered = ImGui::IsWindowHovered();

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
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MODEL_ITEM")) {
                        auto path       = (const wchar_t*)payload->Data;
                        auto wstring    = (std::wstring(path));
                        auto pathString = std::string(begin(wstring), end(wstring));

                        auto model    = Model(pathString);
                        auto filename = pathString.substr(max(pathString.find_last_of('\\'), pathString.find_last_of('/')) + 1);
                        auto e        = m_Renderer->GetScene()->CreateEntity(filename);
                        e.AddComponent<MeshRendererComponent>(model);
                    }
                    ImGui::EndDragDropTarget();
                }

                auto entity = m_SceneHirarchyPanel->GetSelectedEntity();
                if (entity) {
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::SetDrawlist();
                    float w = (float)ImGui::GetWindowWidth(), h = (float)ImGui::GetWindowHeight();

                    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, w, h);

                    auto& transformComponent = entity.GetComponent<TransformComponent>();
                    auto  transform          = transformComponent.GetTransform();

                    ImGuizmo::Manipulate(glm::value_ptr(camera->GetView()), glm::value_ptr(camera->GetProjection()), m_ActiveOperation,
                                         m_ActiveMode, glm::value_ptr(transform));

                    if (ImGuizmo::IsUsing()) {
                        transformComponent.SetTransform(transform);
                    }
                }

                ImGui::End();
                ImGui::PopStyleVar(1);
            }

            {
                // ImGui::Begin("Depth Buffer (Light Space)");

                // ImGui::Image((void*)(intptr_t)m_Renderer->DepthMapID(), ImGui::GetContentRegionAvail(), {0.0f, 1.0f}, {1.0f, 0.0f});

                // ImGui::End();

                // ImGui::Begin("Depth Buffer (View Space)");

                // ImGui::Image((void*)(intptr_t)m_Renderer->SceneDepthMapID(), ImGui::GetContentRegionAvail(), {0.0f, 1.0f},
                //              {1.0f, 0.0f});
                // ImGui::End();

                ImGui::PopStyleVar(1);
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
                            if (ImGui::Selectable(PolygonNames[i].data(), is_selected))
                                config->polygonMode = (PolygonMode)i;
                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
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
                            if (ImGui::Selectable(OperationModes[i].data(), is_selected))
                                m_ActiveMode = (ImGuizmo::MODE)i;
                            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
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
                distanceChange |= ImGui::SliderFloat("Light Camera View Range", &cameraLS->GetViewRange(), 5.0, 100.0f);
                distanceChange |= ImGui::SliderFloat("Light Camera Near", &cameraLS->GetNearClip(), 0, 10);
                distanceChange |= ImGui::SliderFloat("Light Camera Far", &cameraLS->GetFarClip(), 10, 100);
                if (distanceChange)
                    cameraLS->RecalculateProjection();

                // Rotate Directional Light
                float        radius = 10;
                static float time   = 0;
                if (m_Play)
                    time += m_DeltaTime * 0.2;

                cameraLS->GetPosition() = vec3(radius * sin(time), cameraLS->GetPosition().y, radius * cos(time));
                cameraLS->GetForward()  = vec3(0.0f) - cameraLS->GetPosition();
                cameraLS->LookAtWorldCenter();

                // Shadow
                ImGui::Text("Shadow");
                ImGui::Checkbox("Cast Shadow", &config->lightSetting.castShadow);

                // Skybox and IBL ambient
                ImGui::Text("Ambient");
                ImGui::Checkbox("Enable Environment Map", &config->lightSetting.useEnvMap);

                ImGui::End();
            }

            {
                ImGui::Begin("Post FX");
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
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
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

                    {
                        ImGui::Text("Fog");
                        std::vector<std::string> fogTypeNames{"None", "Linear", "Exponential", "Exponential2"};

                        int index = (int)config->postprocessSetting.fogType;
                        if (ImGui::BeginCombo("Fog Function", fogTypeNames[index].data(), 0)) {
                            for (int i = 0; i < fogTypeNames.size(); i++) {
                                const bool is_selected = (index == i);
                                if (ImGui::Selectable(fogTypeNames[i].data(), is_selected))
                                    config->postprocessSetting.fogType = (FogType)i;
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        if (config->postprocessSetting.fogType == FogType::Linear) {
                            auto& start = config->postprocessSetting.fogStart;
                            auto& end   = config->postprocessSetting.fogEnd;
                            widget::ItemLabel("Fog Start");
                            ImGui::DragFloat("##FogStart", &start, 5.0f, 0.0f, end);
                            widget::ItemLabel("Fog End");
                            ImGui::DragFloat("##FogEnd", &end, 5.0f, start, 10000.0f);
                        }
                        ImGui::SliderFloat("Fog Density", &config->postprocessSetting.fogDensity, 0.0f, 1.0f);
                    }

                    {
                        ImGui::Text("Depth of Field");
                        ImGui::Checkbox("Enable DoF", &config->postprocessSetting.enableDoF);
                    }

                    {
                        ImGui::Text("SSAO");
                        ImGui::Checkbox("Enable SSAO", &config->postprocessSetting.enableSSAO);
                    }
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
            for (auto panel : m_Panels)
                panel->OnUIRender();

            if (m_ShowDemoWindow)
                ImGui::ShowDemoWindow();

            m_Renderer->OnUIRender();
        }

        virtual void OnResize(uint32_t w, uint32_t h) override
        {
            if (w == m_ViewportWidth && h == m_ViewportHeight)
                return;
            m_ViewportWidth  = w;
            m_ViewportHeight = h;

            m_Renderer->OnResize(w, h);
            m_Camera->OnResize(w, h);
        }

        void OnEvent()
        {
            if (ImGuizmo::IsUsingAny())
                return;
            if (Input::IsMouseButtonDown(MouseButton::Right))
                return;
            if (Input::IsKeyDown(KeyCode::W))
                m_ActiveOperation = ImGuizmo::OPERATION::TRANSLATE;
            if (Input::IsKeyDown(KeyCode::E))
                m_ActiveOperation = ImGuizmo::OPERATION::SCALE;
            if (Input::IsKeyDown(KeyCode::R))
                m_ActiveOperation = ImGuizmo::OPERATION::ROTATE;
            if (ImGui::IsMouseClicked(0)) {
                // if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyDown(Key::LeftAlt))
                if (m_ViewportHovered)
                    m_SceneHirarchyPanel->SetSelectedEntity(m_HoveredEntity);
            }
            // if (Input::IsMouseButtonDown(MouseButton::Left) && ImGui::IsWindowHovered()) m_Context->activeEntity = -1;
        }

        void OpenScene();
        void OpenScene(std::filesystem::path const& path);

    private:
        std::shared_ptr<Renderer> m_Renderer      = nullptr;
        uint32_t                  m_ViewportWidth = 1920, m_ViewportHeight = 1080;

        std::shared_ptr<Camera> m_Camera = nullptr;

        bool m_ShowDemoWindow  = false;
        bool m_Play            = true;
        bool m_ViewportHovered = false;

        std::shared_ptr<SceneHirarchyPanel> m_SceneHirarchyPanel;
        std::vector<std::shared_ptr<Panel>> m_Panels;

        std::shared_ptr<SceneSerializer> m_SceneSerilizer;

        float m_LastTime  = 0.0f;
        float m_DeltaTime = 0.0f;

        glm::vec2 m_ViewportBounds[2];

        Entity m_HoveredEntity{};

        std::shared_ptr<RuntimeContext> m_Context = nullptr;

        ImGuizmo::OPERATION m_ActiveOperation = ImGuizmo::OPERATION::TRANSLATE;
        ImGuizmo::MODE      m_ActiveMode      = ImGuizmo::MODE::LOCAL;
    };
}  // namespace suplex