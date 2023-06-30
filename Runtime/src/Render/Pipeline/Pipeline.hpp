#pragma once

#include "Render/RenderPass/RenderPass.hpp"
#include <memory>
#include <vector>

namespace suplex {
    class Pipeline {
    public:
        void Attach(const std::shared_ptr<RenderPass> renderPass) { m_RenderPassQueue.emplace_back(renderPass); }

        void Bind() {}
        void Unbind() {}

    private:
        std::vector<std::shared_ptr<RenderPass>> m_RenderPassQueue;
    };
}  // namespace suplex