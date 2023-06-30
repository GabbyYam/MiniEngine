#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "glm/common.hpp"
#include "glm/glm.hpp"
#include <string_view>
#include <iterator>
#include "ImGuizmo.h"

namespace suplex {
    namespace widget {
        enum ItemLabelFlag {
            Left    = 1u << 0u,
            Right   = 1u << 1u,
            Default = Left,
        };

        void ItemLabel(std::string const& title, ItemLabelFlag flag);
        void Vec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);
        void EditTransform(float* cameraView, float* cameraProjection, ImGuizmo::OPERATION& operation, ImGuizmo::MODE& mode);
    }  // namespace widget
}  // namespace suplex