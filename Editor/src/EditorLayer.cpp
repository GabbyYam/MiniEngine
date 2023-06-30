#include "EditorLayer.hpp"
#include "Scene/Scene.hpp"
#include <filesystem>
#include <memory>

namespace suplex {
    void EditorLayer::OpenScene()
    {
        // m_Renderer->GetScene() = std::make_shared<Scene>();
    }

    void EditorLayer::OpenScene(std::filesystem::path const& path)
    {
        std::string filename = path.filename().string();
        if (path.empty() || filename.substr(filename.find_last_of('.') + 1) != "suplex") return;
        m_Renderer->GetScene() = std::make_shared<Scene>();
        m_SceneSerilizer->SetContext(m_Renderer->GetScene());
        m_SceneSerilizer->Deserialize(path.string());
    }
}  // namespace suplex