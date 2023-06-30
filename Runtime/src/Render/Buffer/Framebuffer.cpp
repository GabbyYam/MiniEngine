#include "Framebuffer.hpp"
#include "Render/Buffer/Buffer.hpp"
#include "Render/Buffer/Framebuffer.hpp"
#include <stdexcept>
#include <stdint.h>
#include "glad/glad.h"
#include "spdlog/spdlog.h"

namespace suplex {

    enum class FramebufferTextureFormat { None = 0, RGB, RGBA, DEPTH24STENCIL8, Depth = DEPTH24STENCIL8, RED_INTEGER };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format) : TextureFormat(format) {}

        FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
        // TODO: filtering/wrap
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
            : Attachments(attachments)
        {
        }

        std::vector<FramebufferTextureSpecification> Attachments;
    };

    struct FramebufferSpecification
    {
        uint32_t                           Width = 0, Height = 0;
        FramebufferAttachmentSpecification Attachments;
        uint32_t                           Samples = 1;

        bool SwapChainTarget = false;
    };

    Framebuffer::Framebuffer(uint32_t w, uint32_t h) : Buffer(w, h) { spdlog::info("Framebuffer id assign to {}", m_BufferID); }

    void Framebuffer::OnResize(uint32_t w, uint32_t h)
    {
        if (w == m_Width && h == m_Height) return;
        m_Width = w, m_Height = h;

        // Bind to self
        glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);

        // Generate Framebuffer Image texture
        glDeleteTextures(1, &m_BufferTextureID);
        glGenTextures(1, &m_BufferTextureID);

        // if (m_BufferTextureID == 0) { glGenTextures(1, &m_BufferTextureID); }

        glBindTexture(GL_TEXTURE_2D, m_BufferTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
        spdlog::info("Framebuffer Texture id assign to {}", m_BufferTextureID);

        // Bind texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BufferTextureID, 0);

        // Depth Attachment for framebuffer
        glDeleteRenderbuffers(1, &m_DepthAttachMentID);
        glGenRenderbuffers(1, &m_DepthAttachMentID);

        // if (m_DepthAttachMentID == 0) { glGenTextures(1, &m_DepthAttachMentID); }

        glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
        glBindRenderbuffer(GL_RENDERBUFFER, m_DepthAttachMentID);
        // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_Width, m_Height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachMentID);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachMentID);

        spdlog::info("Framebuffer Depth Renderbuffer id assign to {}", m_DepthAttachMentID);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            spdlog::error("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
            return;
        }

        glViewport(0, 0, m_Width, m_Height);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Return Entity ID in the scene
    int Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
    {
        if (attachmentIndex < m_ColorAttachments.size()) {
            spdlog::error("out of bound");
            return -1;
        }

        this->Bind();

        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
        int pixelData;
        glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
        return pixelData;
    }

}  // namespace suplex