#pragma once

#include "Render/Buffer/Buffer.hpp"
#include <stdint.h>
#include <vector>
#include <cassert>

namespace suplex {
    class Framebuffer : public Buffer {
    public:
        Framebuffer() = default;
        Framebuffer(uint32_t w, uint32_t h);
        virtual ~Framebuffer(){};
        virtual void OnResize(uint32_t w, uint32_t h) override;

        uint32_t GetRenderbufferID() const { return m_DepthAttachMentID; }

        uint32_t GetColorbufferID(uint32_t index)
        {
            assert(index < m_ColorAttachments.size());
            return m_ColorAttachments[index];
        }

        int ReadPixel(uint32_t attachmentIndex, int x, int y);

    private:
        uint32_t              m_DepthAttachMentID = 0;
        std::vector<uint32_t> m_ColorAttachments;
    };
}  // namespace suplex