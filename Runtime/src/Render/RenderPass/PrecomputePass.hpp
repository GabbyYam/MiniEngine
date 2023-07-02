#pragma once
#include <spdlog/spdlog.h>
#include <stdint.h>

#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Render/Buffer/Framebuffer.hpp"
#include "Render/Geometry/Shape/Shape.hpp"
#include "Render/RenderPass/RenderPass.hpp"
#include "Render/Texture/CubeMap.hpp"
#include "Render/Texture/Texture.hpp"
#include "Render/Texture/Texture2D.hpp"


static const glm::mat4 captureProjection =
    glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
static const glm::mat4 captureViews[] = {
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                glm::vec3(0.0f, -1.0f, 0.0f))};

namespace suplex {
class PrecomputePass : public RenderPass {
 public:
  virtual void Render(
      const std::shared_ptr<Camera> camera, const std::shared_ptr<Scene> scene,
      const std::shared_ptr<GraphicsContext> graphicsContext,
      const std::shared_ptr<PrecomputeContext> context) override {
    // Bake to cubemap
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer->GetID());
    glBindRenderbuffer(GL_RENDERBUFFER, m_Framebuffer->GetRenderbufferID());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 2048, 2048);
    glViewport(0, 0, 2048, 2048);

    auto& shader = m_Shaders[0];
    shader->Bind();
    shader->SetMaterix4("proj", glm::value_ptr(captureProjection));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, context->HDR_EnvironmentTexture.GetID());

    for (uint32_t i = 0; i < 6; ++i) {
      shader->SetMaterix4("view", glm::value_ptr(captureViews[i]));
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             context->EnvironmentMap.GetID(), 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      utils::RenderCube(shader);
    }

    shader = m_Shaders[1];
    shader->Bind();
    shader->SetMaterix4("proj", glm::value_ptr(captureProjection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, context->HDR_EnvironmentTexture.GetID());

    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer->GetID());
    glBindRenderbuffer(GL_RENDERBUFFER, m_Framebuffer->GetRenderbufferID());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 32, 32);
    glViewport(0, 0, 32, 32);

    for (uint32_t i = 0; i < 6; ++i) {
      shader->SetMaterix4("view", glm::value_ptr(captureViews[i]));
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             context->IrradianceMap.GetID(), 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      utils::RenderCube(shader);
    }
    shader->Unbind();

    shader = m_Shaders[2];
    shader->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, context->EnvironmentMap.GetID());

    shader->SetMaterix4("proj", glm::value_ptr(captureProjection));
    uint32_t maxMipLevels = 6;
    for (uint32_t mip = 0; mip < maxMipLevels; ++mip) {
      // reisze framebuffer according to mip-level size.
      uint32_t mipWidth = 128 * std::pow(0.5, mip);
      uint32_t mipHeight = 128 * std::pow(0.5, mip);

      glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer->GetID());
      glBindRenderbuffer(GL_RENDERBUFFER, m_Framebuffer->GetRenderbufferID());
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, mipWidth,
                            mipHeight);
      glViewport(0, 0, mipWidth, mipHeight);

      float roughness = (float)mip / (float)(maxMipLevels - 1);
      shader->SetFloat("roughness", &roughness);

      for (uint32_t i = 0; i < 6; ++i) {
        shader->SetMaterix4("view", glm::value_ptr(captureViews[i]));
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               context->PrefilterMap.GetID(), mip);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        utils::RenderCube(shader);
      }
    }
    shader->Unbind();

    shader = m_Shaders[3];
    // then re-configure capture framebuffer object and render screen-space quad
    // with BRDF shader.
    shader->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, context->BRDF_LUT.GetID());
    glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer->GetID());
    glBindRenderbuffer(GL_RENDERBUFFER, m_Framebuffer->GetRenderbufferID());
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           context->BRDF_LUT.GetID(), 0);
    glViewport(0, 0, 512, 512);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    utils::RenderQuad(shader, QuadRenderSpecification::Screen);
    shader->Unbind();

    // Return to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  virtual void Render(uint32_t framebufferID) override {}

  virtual void OnResize(uint32_t w, uint32_t h) override {}

 private:
};
}  // namespace suplex