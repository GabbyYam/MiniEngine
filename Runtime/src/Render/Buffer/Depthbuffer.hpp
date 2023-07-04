#pragma once

#include "Render/Buffer/Buffer.hpp"
#include <stdint.h>
namespace suplex {
    class Depthbuffer : public Buffer {
    public:
        Depthbuffer() = default;
        Depthbuffer(uint32_t w, uint32_t h);
        virtual ~Depthbuffer(){};
        virtual void OnResize(uint32_t w, uint32_t h) override;

    private:
        uint32_t m_gbufferPosition     = 0;
        uint32_t m_gbufferNormal       = 0;
        uint32_t m_DepthRenderbufferID = 0;
    };
}  // namespace suplex