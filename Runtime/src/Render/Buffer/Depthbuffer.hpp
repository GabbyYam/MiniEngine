#pragma once

#include "Render/Buffer/Buffer.hpp"
namespace suplex {
    class Depthbuffer : public Buffer {
    public:
        Depthbuffer() = default;
        Depthbuffer(uint32_t w, uint32_t h);
        virtual ~Depthbuffer(){};
        virtual void OnResize(uint32_t w, uint32_t h) override;
    };
}  // namespace suplex