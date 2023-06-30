#pragma once

#include "Render/Shader/Shader.hpp"
#include "glm/glm.hpp"
#include <algorithm>
#include <memory>
#include <spdlog/spdlog.h>
#include <stdint.h>
#include <vector>
#include "glm/ext.hpp"
#include "Render/Geometry/Shape/Shape.hpp"

namespace suplex {

    struct MipBloom
    {
        uint32_t   textureID;
        glm::vec2  size;
        glm::ivec2 isize;
    };

    class Bloom {
    public:
        Bloom(uint32_t w, uint32_t h, uint32_t mipChainsLength)
        {
            m_Width = w, m_Height = h;
            m_Resolution = glm::vec2(w, h);
            info("Apply Bloom window size = ({}, {})", m_Width, m_Height);

            glGenFramebuffers(1, &m_FramebufferID);
            glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

            glm::vec2  mipSize((float)w, (float)h);
            glm::ivec2 mipIntSize((int)w, (int)h);

            for (unsigned int i = 0; i < mipChainsLength; i++) {
                MipBloom mip;

                mipSize *= 0.5f;
                mipIntSize /= 2;
                mip.size  = mipSize;
                mip.isize = mipIntSize;

                glGenTextures(1, &mip.textureID);
                glBindTexture(GL_TEXTURE_2D, mip.textureID);
                // we are downscaling an HDR color buffer, so we need a float texture format
                glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, (int)mipSize.x, (int)mipSize.y, 0, GL_RGB, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                m_MipChains.emplace_back(mip);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_MipChains[0].textureID, 0);

            // setup attachments
            unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, attachments);

            // check completion status
            int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                spdlog::error("gbuffer FBO error, status: 0x\%x\n", status);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                return;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            m_DownsampleShader = std::make_shared<Shader>("quad.vert", "bloom_downsample.frag");
            m_UpsampleShader   = std::make_shared<Shader>("quad.vert", "bloom_upsample.frag");

            m_DownsampleShader->Bind();
            m_DownsampleShader->SetInt("srcTexture", 0);

            m_UpsampleShader->Bind();
            m_UpsampleShader->SetInt("srcTexture", 0);
        }

        ~Bloom()
        {
            for (int i = 0; i < m_MipChains.size(); i++) {
                glDeleteTextures(1, &m_MipChains[i].textureID);
                m_MipChains[i].textureID = 0;
            }
            glDeleteFramebuffers(1, &m_FramebufferID);
            m_FramebufferID = 0;
        }

        void Render(uint32_t srcTextureID, float radius)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
            glViewport(0, 0, m_Width, m_Height);
            RenderDownsample(srcTextureID);
            RenderUpsample(radius);
        }

        void RenderDownsample(uint32_t srcTextureID)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
            m_DownsampleShader->Bind();
            m_DownsampleShader->SetFloat2("resolution", glm::value_ptr(m_Resolution));

            // Bind srcTexture (HDR color buffer) as initial texture input
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, srcTextureID);

            // Progressively downsample through the mip chain
            for (int i = 0; i < m_MipChains.size(); i++) {
                auto& mip = m_MipChains[i];
                glViewport(0, 0, mip.size.x, mip.size.y);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.textureID, 0);

                // Render screen-filled quad of resolution of current mip
                utils::RenderQuad(m_DownsampleShader);

                // Set current mip resolution as srcResolution for next iteration
                m_DownsampleShader->SetFloat2("resolution", glm::value_ptr(mip.size));
                // Set current mip as texture input for next iteration
                glBindTexture(GL_TEXTURE_2D, mip.textureID);
            }

            m_DownsampleShader->Unbind();
        }

        void RenderUpsample(float radius)
        {
            m_UpsampleShader->Bind();
            m_UpsampleShader->SetFloat("filterRadius", &radius);

            // Enable additive blending
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glBlendEquation(GL_FUNC_ADD);

            for (int i = m_MipChains.size() - 1; i > 0; i--) {
                const auto& mip     = m_MipChains[i];
                const auto& nextMip = m_MipChains[i - 1];

                // Bind viewport and texture from where to read
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mip.textureID);

                // Set framebuffer render target (we write to this texture)
                glViewport(0, 0, nextMip.size.x, nextMip.size.y);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.textureID, 0);

                // Render screen-filled quad of resolution of current mip
                utils::RenderQuad(m_UpsampleShader);
            }

            // Disable additive blending
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);  // Restore if this was default
            glDisable(GL_BLEND);

            m_UpsampleShader->Unbind();
        }

        uint32_t GetResultID() { return m_MipChains[0].textureID; }

    private:
        uint32_t                m_FramebufferID;
        std::vector<MipBloom>   m_MipChains;
        std::shared_ptr<Shader> m_DownsampleShader = nullptr;
        std::shared_ptr<Shader> m_UpsampleShader   = nullptr;

        uint32_t  m_Width = 0, m_Height = 0;
        glm::vec2 m_Resolution{0.0f};
    };
}  // namespace suplex