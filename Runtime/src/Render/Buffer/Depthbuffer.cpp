#include "Render/Buffer/Depthbuffer.hpp"
#include "Render/Buffer/Buffer.hpp"
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include "glad/glad.h"

namespace suplex {
    Depthbuffer::Depthbuffer(uint32_t w, uint32_t h) : Buffer(w, h) { spdlog::info("DepthBuffer id assign to {}", m_BufferID); }

    void Depthbuffer::OnResize(uint32_t w, uint32_t h)
    {
        if (w == m_Width && h == m_Height)
            return;
        m_Width = w, m_Height = h;
        glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);

        // Bind texture to depthbuffer
        glDeleteTextures(1, &m_BufferTextureID);
        glGenTextures(1, &m_BufferTextureID);

        glBindTexture(GL_TEXTURE_2D, m_BufferTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        spdlog::info("DepthMap id assign to {}", m_BufferTextureID);

        // Clamp outof screen depth to 1.0
        GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_BufferTextureID, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            spdlog::error("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
            return;
        }
        glViewport(0, 0, m_Width, m_Height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}  // namespace suplex