#include <iostream>
#include "Panel/SceneHirarchyPanel.hpp"
#include "Scene/Component/Component.hpp"
#include "Scene/Scene.hpp"
#include "Scene/Entity/Entity.hpp"
#include "imgui.h"
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include "glm/glm.hpp"
#include <ImGuizmo.h>
#include "Widget/CommonWidget.hpp"
#include <IconsFontAwesome6.h>
#include <Scene/Entity/Entity.hpp>

namespace suplex {
    void SceneHirarchyPanel::OnUIRender()
    {
        auto& renderer = m_Context->renderer;
        auto& scene    = m_Context->renderer->GetScene();
        auto& registry = m_Context->renderer->GetScene()->m_Registry;
        {
            // ImGui::Begin(ICON_FA_PROJECT_DIAGRAM "  Hierarchy");
            ImGui::Begin("Hierarchy");
            auto entities = registry.view<MeshRendererComponent, TagComponent>();
            for (auto entity : entities) {
                auto& model         = registry.get<MeshRendererComponent>(entity).m_Model;
                auto  tag           = registry.get<TagComponent>(entity).m_Tag;
                int   materialIndex = model->GetMaterialIndex();
                if (ImGui::TreeNode(tag.c_str())) {
                    m_ActiveEntity = Entity(entity, scene.get());
                    ImGui::TreePop();
                }
            }
            ImGui::End();
        }

        {
            ImGui::Begin("Properties");
            {
                auto   v = ImGui::GetContentRegionAvail();
                float  w = v.x * 0.5f;
                ImVec2 t_size{w, w};

                if (m_ActiveEntity) {
                    auto& transform = m_ActiveEntity.GetComponent<TransformComponent>();
                    auto& tag       = m_ActiveEntity.GetComponent<TagComponent>();

                    // auto object = renderer->GetGameObjectList()[activeEntityIndex];

                    widget::ItemLabel(ICON_FA_TAG "  Tag", widget::Left);
                    ImGui::InputText("##Tag", tag.m_Tag.data(), 256);

                    bool active = true;
                    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                        auto& translate = transform.m_Translation;
                        auto& rotation  = transform.m_Rotation;
                        auto& scale     = transform.m_Scale;
                        widget::Vec3Control("Position", translate, 0.0f);
                        widget::Vec3Control("Rotation", rotation, 0.0f);
                        widget::Vec3Control("Scale", scale, 1.0f);
                    }

                    if (m_ActiveEntity.HasComponent<MeshRendererComponent>()) {
                        auto& model = m_ActiveEntity.GetComponent<MeshRendererComponent>();
                        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
                            int materialIndex = model.m_Model->GetMaterialIndex();

                            auto&       shaders     = renderer->GetShadersList();
                            std::string previewName = shaders[materialIndex]->GetShaderName();

                            // ImGui::Text("Shader");
                            widget::ItemLabel("Shader", widget::ItemLabelFlag::Left);

                            if (ImGui::BeginCombo("##Material", previewName.data(), 0)) {
                                for (int i = 0; i < shaders.size(); i++) {
                                    const bool is_selected = (materialIndex == i);
                                    if (ImGui::Selectable(shaders[i]->GetShaderName().data(), is_selected))
                                        model.m_Model->SetMaterialIndex(i);
                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_selected) ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                        }
                    }
                }
            }
            ImGui::End();
        }
    }

    void SceneHirarchyPanel::OnEvent() {}

}  // namespace suplex