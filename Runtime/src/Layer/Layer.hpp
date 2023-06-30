#pragma once

#include <spdlog/spdlog.h>
#include <stdint.h>

namespace suplex {
    class Layer {
    public:
        Layer() = default;
        virtual ~Layer() {}
        virtual void OnResize(uint32_t w, uint32_t h) {}
        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUIRender() {}
        virtual void OnUpdate(float ts) {}
    };
}  // namespace suplex