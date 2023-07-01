#pragma once

#include "Render/Buffer/Buffer.hpp"
#include <stdexcept>
#include <stdint.h>
namespace suplex {
    class HdrFramebuffer : public Buffer {
    public:
        HdrFramebuffer() = default;
        HdrFramebuffer(uint32_t w, uint32_t h);
        virtual ~HdrFramebuffer(){};
        virtual void     OnResize(uint32_t w, uint32_t h) override;
        virtual uint32_t GetTextureID() const override { return m_Colorbuffer[0]; }
        uint32_t         GetTextureID0() const { return m_Colorbuffer[0]; }
        uint32_t         GetTextureID1() const { return m_Colorbuffer[1]; }
        uint32_t         GetTextureID2() const { return m_RedIntegerBuffer; }

        uint32_t GetRenderbufferID() const { return m_DepthAttachMentID; }
        int      ReadPixel(int x, int y);

    private:
        uint32_t m_DepthAttachMentID = 0;
        uint32_t m_Colorbuffer[2];
        uint32_t m_RedIntegerBuffer = 0;
    };
}  // namespace suplex