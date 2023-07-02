#include "CommonWidget.hpp"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
using namespace ImGui;

namespace suplex {
    namespace widget {
        void ItemLabel(std::string const& title, ItemLabelFlag flag)
        {
            ImGuiWindow*      window    = ImGui::GetCurrentWindow();
            const ImVec2      lineStart = ImGui::GetCursorScreenPos();
            const ImGuiStyle& style     = ImGui::GetStyle();
            float             fullWidth = ImGui::GetContentRegionAvail().x;
            float             itemWidth = ImGui::CalcItemWidth() + style.ItemSpacing.x;
            ImVec2            textSize  = ImGui::CalcTextSize(title.c_str(), title.c_str() + title.size());
            ImRect            textRect;
            textRect.Min = ImGui::GetCursorScreenPos();

            if (flag & ItemLabelFlag::Right)
                textRect.Min.x = textRect.Min.x + itemWidth;
            textRect.Max = textRect.Min;
            textRect.Max.x += fullWidth - itemWidth;
            textRect.Max.y += textSize.y;

            ImGui::SetCursorScreenPos(textRect.Min);

            ImGui::AlignTextToFramePadding();
            // Adjust text rect manually because we render it directly into a drawlist instead of using public functions.
            textRect.Min.y += window->DC.CurrLineTextBaseOffset;
            textRect.Max.y += window->DC.CurrLineTextBaseOffset;

            ItemSize(textRect);
            if (ItemAdd(textRect, window->GetID(title.data(), title.data() + title.size()))) {
                RenderTextEllipsis(ImGui::GetWindowDrawList(), textRect.Min, textRect.Max, textRect.Max.x, textRect.Max.x,
                                   title.data(), title.data() + title.size(), &textSize);

                if (textRect.GetWidth() < textSize.x && ImGui::IsItemHovered())
                    ImGui::SetTooltip("%.*s", (int)title.size(), title.data());
            }
            if (flag & ItemLabelFlag::Left) {
                ImGui::SetCursorScreenPos(ImVec2{textRect.Max.x, textRect.Max.y - (textSize.y + window->DC.CurrLineTextBaseOffset)});
                ImGui::SameLine();
            }
            else if (flag && ItemLabelFlag::Right)
                ImGui::SetCursorScreenPos(lineStart);
        }

        void Vec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth)
        {
            ImGuiIO& io       = ImGui::GetIO();
            auto     boldFont = io.Fonts->Fonts[0];

            ImGui::PushID(label.data());

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label.data());
            ImGui::NextColumn();

            ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

            float  lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

            // ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
            // ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
            // ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
            ImGui::PushFont(boldFont);
            if (ImGui::Button(ICON_FA_X, buttonSize))
                values.x = resetValue;
            ImGui::PopFont();
            // ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::InputFloat("##X", &values.x, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();

            // ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
            // ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
            // ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
            ImGui::PushFont(boldFont);
            if (ImGui::Button(ICON_FA_Y, buttonSize))
                values.y = resetValue;
            ImGui::PopFont();
            // ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::InputFloat("##Y", &values.y, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();
            ImGui::SameLine();

            // ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
            // ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
            // ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
            ImGui::PushFont(boldFont);
            if (ImGui::Button(ICON_FA_Z, buttonSize))
                values.z = resetValue;
            ImGui::PopFont();
            // ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::InputFloat("##Z", &values.z, 0.0f, 0.0f, "%.2f");
            ImGui::PopItemWidth();

            ImGui::PopStyleVar();

            ImGui::Columns(1);

            ImGui::PopID();
        }

        void EditTransform(float* cameraView, float* cameraProjection, ImGuizmo::OPERATION& operation, ImGuizmo::MODE& mode)
        {
            // if (ImGui::IsKeyPressed(ImGuiKey_W)) operation = ImGuizmo::TRANSLATE;
            // if (ImGui::IsKeyPressed(ImGuiKey_E)) operation = ImGuizmo::SCALE;
            // if (ImGui::IsKeyPressed(ImGuiKey_R)) operation = ImGuizmo::ROTATE;

            ImGuizmo::SetOrthographic(false);
            float w = (float)ImGui::GetWindowWidth(), h = (float)ImGui::GetWindowHeight();
            float x = ImGui::GetWindowPos().x, y = ImGui::GetWindowPos().y;
            ImGuizmo::SetRect(x, y, w, h);
            ImGuizmo::ViewManipulate(cameraView, 10, ImVec2(x + w - 128, y), ImVec2(128, 128), 0x10101010);
        }

    }  // namespace widget
}  // namespace suplex