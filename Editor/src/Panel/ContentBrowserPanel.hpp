#pragma once
#include "Panel.hpp"
#include "Render/Texture/Texture2D.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <filesystem>
#include <memory>

namespace suplex {
    class ContentBrowserPanel : public Panel {
    public:
        ContentBrowserPanel();

        virtual void OnUIRender() override;
        virtual void OnEvent() override;

    private:
        std::filesystem::path      m_CurrentDirectory;
        std::shared_ptr<Texture2D> m_DirectoryICON;
        std::shared_ptr<Texture2D> m_FileICON;
        std::shared_ptr<Texture2D> m_ReturnICON;
        ImVec2                     m_ICONSIZE = ImVec2{100, 100};
    };
}  // namespace suplex