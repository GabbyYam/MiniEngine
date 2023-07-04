#pragma once

#include "Render/Buffer/Buffer.hpp"
#include <Render/Texture/Texture.hpp>
#include <stdint.h>
#include <vector>
#include <cassert>

namespace suplex {

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(TextureFormat format,
                                        TextureFilter filter = TextureFilter::Linear,
                                        TextureWrap   wrap   = TextureWrap::ClampToEdge)
            : TextureFormat(format), TextureFilter(filter), TextureWrap(wrap)
        {
        }

        TextureFormat TextureFormat = TextureFormat::None;
        TextureFilter TextureFilter = TextureFilter::Linear;
        TextureWrap   TextureWrap   = TextureWrap::ClampToEdge;
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

    class Framebuffer : public Buffer {
    public:
        Framebuffer() = default;
        Framebuffer(FramebufferSpecification const& spec);
        Framebuffer(uint32_t w, uint32_t h);
        virtual ~Framebuffer(){};
        virtual void OnResize(uint32_t w, uint32_t h) override;

        uint32_t GetDepthAttachmentID() const { return m_DepthAttachMentID; }

        uint32_t GetColorAttachmentID(uint32_t index) { return m_ColorAttachments[index]; }

        uint32_t GetRenderbufferID() { return m_DepthRenderbufferID; }

        int ReadPixel(uint32_t attachmentIndex, int x, int y);

    private:
        std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
        std::vector<uint32_t>                        m_ColorAttachments;

        FramebufferTextureSpecification m_DepthAttachmentSpecification;
        uint32_t                        m_DepthAttachMentID = 0;

        bool     m_IsSwapChainTarget   = false;
        uint32_t m_DepthRenderbufferID = 0;
    };
}  // namespace suplex