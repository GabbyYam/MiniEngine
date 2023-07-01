#pragma once

#include <spdlog/spdlog.h>
#include <stdint.h>
#include "Render/Buffer/Buffer.hpp"
#include "glad/glad.h"

namespace suplex {
    class Buffer {
    public:
        Buffer()
        {
            glGenFramebuffers(1, &m_BufferID);
            glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
        }

        Buffer(uint32_t w, uint32_t h) : Buffer() { OnResize(w, h); };

        virtual ~Buffer(){};
        virtual void OnResize(uint32_t w, uint32_t h) {}

        virtual void Bind()
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
            glViewport(0, 0, m_Width, m_Height);
        }

        virtual void Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

        uint32_t         GetID() const { return m_BufferID; }
        virtual uint32_t GetTextureID() const { return m_BufferTextureID; }

    protected:
        uint32_t m_BufferID        = 0;
        uint32_t m_BufferTextureID = 0;
        uint32_t m_Width = 0, m_Height = 0;
    };

}  // namespace suplex