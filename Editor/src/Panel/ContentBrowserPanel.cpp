#include "ContentBrowserPanel.hpp"
#include "Render/Texture/Texture.hpp"
#include "Render/Texture/Texture2D.hpp"
#include "imgui.h"
#include <filesystem>
#include <memory>
#include <setjmp.h>
#include <vcruntime_typeinfo.h>

namespace suplex {
    constexpr char* s_DefaultAssetPath = "../Assets";

    ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(s_DefaultAssetPath)
    {
        m_DirectoryICON =
            std::make_shared<Texture2D>(std::string(s_DefaultAssetPath) + "/Icons/icons8-folder-50.png", ImageFormat::RGBA);
        m_FileICON   = std::make_shared<Texture2D>(std::string(s_DefaultAssetPath) + "/Icons/icons8-file-50.png", ImageFormat::RGBA);
        m_ReturnICON = std::make_shared<Texture2D>(std::string(s_DefaultAssetPath) + "/Icons/icons8-return-50.png", ImageFormat::RGBA);
    }

    void ContentBrowserPanel::OnUIRender()
    {
        ImGui::Begin("Content Browser");
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0, 0, 0, 0});

        if (m_CurrentDirectory != std::filesystem::path(s_DefaultAssetPath)) {
            static float returnSize = 16.0f;
            if (ImGui::ImageButton("##Return", (void*)(intptr_t)m_ReturnICON->GetID(), {returnSize, returnSize})) {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }

        static float padding       = 16.0f;
        static float thumbnailSize = 64.0f;
        float        cellSize      = thumbnailSize + padding;

        float panelWidth  = ImGui::GetContentRegionAvail().x;
        int   columnCount = std::max(1, (int)(panelWidth / cellSize));

        ImGui::Columns(columnCount, 0, false);

        for (auto& it : std::filesystem::directory_iterator(m_CurrentDirectory)) {
            const auto& path     = it.path();
            std::string filename = path.filename().string();
            ImGui::PushID(filename.c_str());
            auto icon = it.is_directory() ? m_DirectoryICON : m_FileICON;

            ImGui::ImageButton(filename.data(), (void*)(intptr_t)icon->GetID(), {thumbnailSize, thumbnailSize});

            if (ImGui::BeginDragDropSource()) {
                std::filesystem::path relativePath(path);
                const wchar_t*        itemPath = relativePath.c_str();
                ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
                ImGui::EndDragDropSource();
            }

            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (it.is_directory()) m_CurrentDirectory /= path.filename();
            }
            ImGui::TextWrapped("%s", filename.data());
            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::PopStyleColor();
        ImGui::End();
    }
    void ContentBrowserPanel::OnEvent() {}
}  // namespace suplex