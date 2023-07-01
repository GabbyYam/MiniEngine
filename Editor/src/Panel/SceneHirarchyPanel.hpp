#pragma once

#include <iostream>
#include "Panel/Panel.hpp"
#include "Scene/Entity/Entity.hpp"
#include "imgui.h"
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include "glm/glm.hpp"
#include <ImGuizmo.h>

namespace suplex {
    class SceneHirarchyPanel : public Panel {
    public:
        SceneHirarchyPanel() = default;

        virtual void OnUIRender() override;
        virtual void OnEvent() override;

        void   SetSelectedEntity(Entity entity) { m_ActiveEntity = entity; }
        Entity GetSelectedEntity() { return m_ActiveEntity; }

        Entity m_ActiveEntity{};
    };
}  // namespace suplex