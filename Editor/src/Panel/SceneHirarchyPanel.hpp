#pragma once

#include <iostream>
#include "Panel/Panel.hpp"
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
    };
}  // namespace suplex