#include "HdrFramebuffer.hpp"
#include "Render/Buffer/Buffer.hpp"
#include "Render/Buffer/HdrFramebuffer.hpp"
#include <stdint.h>
#include "glad/glad.h"
#include "spdlog/spdlog.h"

namespace suplex {
    HdrFramebuffer::HdrFramebuffer(uint32_t w, uint32_t h) : Buffer(w, h)
    {
        spdlog::info("HdrFramebuffer id assign to {}", m_BufferID);
    }

    void HdrFramebuffer::OnResize(uint32_t w, uint32_t h)
    {
        if (w == m_Width && h == m_Height) return;
        m_Width = w, m_Height = h;

        // Bind to self
        glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);

        glDeleteTextures(2, m_Colorbuffer);
        glGenTextures(2, m_Colorbuffer);

        for (unsigned int i = 0; i < 2; i++) {
            // glDeleteBuffers(1, &m_Colorbuffer[i]);
            // glGenBuffers(1, &m_Colorbuffer[i]);

            glBindTexture(GL_TEXTURE_2D, m_Colorbuffer[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // attach texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_Colorbuffer[i], 0);

            glGenerateMipmap(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        {
            glDeleteTextures(1, &m_RedIntegerBuffer);
            glGenTextures(1, &m_RedIntegerBuffer);
            glBindTexture(GL_TEXTURE_2D, m_RedIntegerBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_Width, m_Height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // attach texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 2, GL_TEXTURE_2D, m_RedIntegerBuffer, 0);

            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // Depth Attachment for framebuffer
        {
            glDeleteRenderbuffers(1, &m_DepthAttachMentID);
            glGenRenderbuffers(1, &m_DepthAttachMentID);

            glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
            glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachMentID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachMentID);
            spdlog::info("HdrFramebuffer Depth Renderbuffer id assign to {}", m_DepthAttachMentID);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            spdlog::error("ERROR::FRAMEBUFFER:: HdrFramebuffer is not complete!");
            return;
        }
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);

        glViewport(0, 0, m_Width, m_Height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    int HdrFramebuffer::ReadPixel(int x, int y)
    {
        // this->Bind();
        glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + 2);

        int pixelData = -1;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
        return pixelData;
    }

}  // namespace suplex