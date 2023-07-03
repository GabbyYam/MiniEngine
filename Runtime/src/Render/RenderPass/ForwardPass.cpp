#include "ForwardPass.hpp"
#include "Render/Geometry/Shape/Shape.hpp"
#include "Render/RenderPass/ForwardPass.hpp"
#include "Render/Texture/Texture2D.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace suplex {
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f, 10.0f, 10.0f),
        glm::vec3(10.0f, 10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f),
    };

    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
    };

    void ForwardRenderPass::Bind(const std::shared_ptr<Camera>            camera,
                                 const std::shared_ptr<GraphicsContext>   graphicsContext,
                                 const std::shared_ptr<PrecomputeContext> context)
    {
        m_Framebuffer->Bind();
        glCullFace(GL_BACK);
        auto& config  = graphicsContext->config;
        auto& shaders = m_Shaders;
        for (auto& shader : shaders) {
            // // Bind Depth buffer to forward sampler
            shader->Bind();

            shader->SetInt("useEnvMap", config->lightSetting.useEnvMap);

            shader->BindTexture("DepthMap", graphicsContext->depthMapLS, 15, SamplerType::Texture2D);

            shader->BindTexture("IrradianceMap", context->IrradianceMap.GetID(), 14, SamplerType::CubeMap);

            shader->BindTexture("PrefilterMap", context->PrefilterMap.GetID(), 13, SamplerType::CubeMap);

            shader->BindTexture("BRDF_LUT", context->BRDF_LUT.GetID(), 12, SamplerType::Texture2D);

            // Light position
            for (unsigned int i = 0; i < 4; ++i) {
                shader->SetFloat3("lightPositions[" + std::to_string(i) + "]", glm::value_ptr(lightPositions[i]));
                shader->SetFloat3("lightColors[" + std::to_string(i) + "]", glm::value_ptr(lightColors[i]));
            }

            for (unsigned int i = 0; i < 100; ++i) {
                shader->SetMaterix4("boneTransform[" + std::to_string(i) + "]", glm::value_ptr(glm::mat4(1.0f)));
            }

            shader->Unbind();
        }
    }

    void ForwardRenderPass::RenderLight(const std::shared_ptr<Camera>            camera,
                                        const std::shared_ptr<GraphicsContext>   graphicsContext,
                                        const std::shared_ptr<PrecomputeContext> context)
    {
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        const auto& view   = camera->GetView();
        const auto& proj   = camera->GetProjection();
        auto        config = graphicsContext->config;
        // Render Directional Light
        {
            auto& lightShader = m_Shaders[3];
            lightShader->Bind();
            // Light Setting
            lightShader->SetFloat3("lightDirection", glm::value_ptr(config->lightSetting.cameraLS->GetForward()));
            lightShader->SetFloat3("lightPosition", glm::value_ptr(config->lightSetting.cameraLS->GetPosition()));
            lightShader->SetFloat("lightIntensity", &config->lightSetting.lightIntensity);
            lightShader->SetFloat3("lightColor", glm::value_ptr(config->lightSetting.lightColor));

            // Camera Position
            lightShader->SetFloat3("viewPos", glm::value_ptr(camera->GetPosition()));

            // MVP & Light MVP
            auto model = glm::translate(glm::mat4(1.0f), config->lightSetting.cameraLS->GetPosition());
            model      = glm::scale(model, vec3(0.5f));

            lightShader->SetMaterix4("model", glm::value_ptr(model));
            lightShader->SetMaterix4("view", glm::value_ptr(view));
            lightShader->SetMaterix4("proj", glm::value_ptr(proj));

            utils::RenderSphere(lightShader);
            lightShader->Unbind();
        }

        // Point Light
        {
            m_IconShader->Bind();
            for (int i = 0; i < 4; ++i) {
                glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
                newPos           = lightPositions[i];
                auto model       = glm::translate(glm::mat4(1.0f), newPos);
                m_IconShader->SetMaterix4("model", glm::value_ptr(model));
                m_IconShader->SetMaterix4("view", glm::value_ptr(view));
                m_IconShader->SetMaterix4("proj", glm::value_ptr(proj));
                m_IconShader->BindTexture("albedo", m_LightIcon->GetID(), 0, SamplerType::Texture2D);
                utils::RenderQuad(m_IconShader, QuadRenderSpecification::Texture2D);
            }
        }

        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);
    }

    void ForwardRenderPass::RenderOutline(const std::shared_ptr<Camera>            camera,
                                          const std::shared_ptr<GraphicsContext>   graphicsContext,
                                          const std::shared_ptr<PrecomputeContext> context)
    {
        const auto& view = camera->GetView();
        const auto& proj = camera->GetProjection();
        // Render Stencil for active entity
        if (auto entity = graphicsContext->activeEntity) {
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);

            m_OutlineShader->Bind();
            auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();

            m_OutlineShader->SetInt("entityID", static_cast<int>(entity.GetID()));

            // MVP & Light MVP
            auto transform = entity.GetComponent<TransformComponent>();
            transform.m_Scale *= vec3(1.01);
            m_OutlineShader->SetMaterix4("model", glm::value_ptr(transform.GetTransform()));
            m_OutlineShader->SetMaterix4("view", glm::value_ptr(view));
            m_OutlineShader->SetMaterix4("proj", glm::value_ptr(proj));

            for (auto& mesh : meshRenderer.m_Model->GetMeshes())
                mesh.Render(m_OutlineShader);
            m_OutlineShader->Unbind();

            glStencilMask(0xFF);
            glDisable(GL_STENCIL_TEST);
        }
    }
}  // namespace suplex