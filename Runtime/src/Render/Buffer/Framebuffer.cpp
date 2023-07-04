#include "Framebuffer.hpp"
#include "Render/Buffer/Buffer.hpp"
#include "Render/Buffer/Framebuffer.hpp"
#include <gl/gl.h>
#include <stdexcept>
#include <stdint.h>
#include <vector>
#include "Render/Texture/Texture.hpp"
#include "glad/glad.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <numeric>

namespace suplex {

    Framebuffer::Framebuffer(uint32_t w, uint32_t h) : Buffer(w, h) { spdlog::info("Framebuffer id assign to {}", m_BufferID); }

    Framebuffer::Framebuffer(FramebufferSpecification const& spec) : Buffer(), m_IsSwapChainTarget(spec.SwapChainTarget)
    {
        for (auto attachmentSpec : spec.Attachments.Attachments) {
            if (attachmentSpec.TextureFormat == TextureFormat::Depth) {
                m_DepthAttachmentSpecification = attachmentSpec;
            }
            else {
                m_ColorAttachmentSpecifications.push_back(attachmentSpec);
                m_ColorAttachments.push_back(0);
            }
        }
    }

    void Framebuffer::OnResize(uint32_t w, uint32_t h)
    {
        if (w == m_Width && h == m_Height)
            return;
        m_Width = w, m_Height = h;
        spdlog::info("Framebuffer {} Resize to ({}, {})", m_BufferID, w, h);

        // Bind to self
        glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);

        // Color Attachments
        for (int i = 0; i < m_ColorAttachmentSpecifications.size(); ++i) {
            glDeleteTextures(1, &m_ColorAttachments[i]);
            glGenTextures(1, &m_ColorAttachments[i]);

            glBindTexture(GL_TEXTURE_2D, m_ColorAttachments[i]);
            auto& spec = m_ColorAttachmentSpecifications[i];
            switch (spec.TextureFilter) {
                case TextureFilter::Linear:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    break;
                case TextureFilter::Nearest:
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }

            switch (spec.TextureWrap) {
                case TextureWrap::ClampToEdge: {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    break;
                }
                case TextureWrap::ClampToBorder: {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                    GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
                    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
                    break;
                }
                case TextureWrap::Repeat: {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    break;
                }
            }

            switch (spec.TextureFormat) {
                case TextureFormat::RGB:
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, NULL);
                    break;
                case TextureFormat::RGBA:
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, NULL);
                    break;
                case TextureFormat::RED_INTEGER:
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32I, m_Width, m_Height, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
                    break;
                default: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, NULL); break;
            }

            // Bind texture to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_ColorAttachments[i], 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        if (m_ColorAttachments.size()) {
            std::vector<uint32_t> drawAttachments(m_ColorAttachments.size());
            std::iota(begin(drawAttachments), end(drawAttachments), GL_COLOR_ATTACHMENT0);
            glDrawBuffers(drawAttachments.size(), drawAttachments.data());
        }

        // Depth Attachment
        if (m_DepthAttachmentSpecification.TextureFormat != TextureFormat::None) {
            // Bind texture to depthbuffer
            glDeleteTextures(1, &m_DepthAttachMentID);
            glGenTextures(1, &m_DepthAttachMentID);

            glBindTexture(GL_TEXTURE_2D, m_DepthAttachMentID);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            // Clamp outof screen depth to 1.0
            GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            glBindTexture(GL_TEXTURE_2D, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachMentID, 0);
        }

        {
            glDeleteRenderbuffers(1, &m_DepthRenderbufferID);
            glGenRenderbuffers(1, &m_DepthRenderbufferID);

            if (m_IsSwapChainTarget) {
                glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
                glBindRenderbuffer(GL_RENDERBUFFER, m_DepthRenderbufferID);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthRenderbufferID);
                glBindFramebuffer(GL_FRAMEBUFFER, m_BufferID);
            }
        }

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
        if (attachmentIndex >= m_ColorAttachments.size()) {
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