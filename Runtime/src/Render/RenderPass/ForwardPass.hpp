#pragma once
#include "Render/Config/Config.hpp"
#include "Render/Geometry/Mesh.hpp"
#include "Render/Geometry/Shape/Shape.hpp"
#include "Render/Shader/Shader.hpp"
#include "Render/Texture/Texture.hpp"
#include "Render/Texture/Texture2D.hpp"
#include "RenderPass.hpp"
#include "Scene/Component/Component.hpp"
#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <Scene/Scene.hpp>
#include <optional>
#include <vector>
#include <Scene/Entity/Entity.hpp>

extern suplex::Texture2D solidWhite;
extern glm::vec3         lightColors[];
extern glm::vec3         lightPositions[];

namespace suplex {

    class ForwardRenderPass : public RenderPass {
    public:
        ForwardRenderPass()
        {
            PushShader(std::make_shared<Shader>("common.vert", "pbr.frag"));
            PushShader(std::make_shared<Shader>("common.vert", "phong.frag"));
            PushShader(std::make_shared<Shader>("toon.vert", "toon.frag"));
            PushShader(std::make_shared<Shader>("common.vert", "light.frag"));
        }

        virtual void Render(const std::shared_ptr<Camera>            camera,
                            const std::shared_ptr<Scene>             scene,
                            const std::shared_ptr<GraphicsContext>   graphicsContext,
                            const std::shared_ptr<PrecomputeContext> context) override
        {
            // Enable stencil test
            // ------

            Bind(camera, graphicsContext, context);

            static int value = -1;
            glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer->GetID());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glClearTexImage(m_Framebuffer->GetColorAttachmentID(2), 0, GL_RED_INTEGER, GL_INT, &value);
            glDisable(GL_STENCIL_TEST);
            // =====================================================

            auto config = graphicsContext->config;
            glEnable(GL_CULL_FACE);

            auto view = camera->GetView();
            auto proj = camera->GetProjection();

            auto& lightCamera = config->lightSetting.cameraLS;
            auto  viewLS      = lightCamera->GetView();
            auto  projLS      = lightCamera->GetProjection();

            auto mvpLS = projLS * viewLS;

            m_GridShader->Bind();
            m_GridShader->SetMaterix4("view", glm::value_ptr(view));
            m_GridShader->SetMaterix4("proj", glm::value_ptr(proj));
            utils::RenderGird(nullptr);
            m_GridShader->Unbind();

            // render container
            auto DrawEntity = [&](Entity entity, std::optional<std::shared_ptr<Shader>> effectShader = std::nullopt) {
                auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();

                auto shader = effectShader.value_or(m_Shaders[meshRenderer.m_Model->GetMaterialIndex()]);
                shader->Bind();
                shader->SetInt("entityID", static_cast<int>(entity.GetID()));
                // Light Setting
                shader->SetFloat3("lightDirection", glm::value_ptr(config->lightSetting.cameraLS->GetForward()));
                shader->SetFloat3("lightPosition", glm::value_ptr(config->lightSetting.cameraLS->GetPosition()));
                shader->SetFloat("lightIntensity", &config->lightSetting.lightIntensity);
                shader->SetFloat3("lightColor", glm::value_ptr(config->lightSetting.lightColor));

                // For postprocessing
                shader->SetFloat("bloomThreshold", &config->postprocessSetting.bloomThreshold);

                // Camera Position
                shader->SetFloat3("viewPos", glm::value_ptr(camera->GetPosition()));

                // MVP & Light MVP
                auto transform = entity.GetComponent<TransformComponent>();
                shader->SetMaterix4("model", glm::value_ptr(transform.GetTransform()));
                shader->SetMaterix4("view", glm::value_ptr(view));
                shader->SetMaterix4("proj", glm::value_ptr(proj));
                shader->SetMaterix4("mvpLS", glm::value_ptr(mvpLS));

                // material parameters
                shader->SetFloat("baseF", config->pbrSetting.baseF);
                shader->SetFloat3("baseColor", glm::value_ptr(config->pbrSetting.baseColor));
                shader->SetFloat("metallic", &config->pbrSetting.metallic);
                shader->SetFloat("roughness", &config->pbrSetting.roughness);
                shader->SetFloat("ao", &config->pbrSetting.ao);
                shader->SetInt("kullaConty", config->pbrSetting.enableKullaConty);

                shader->BindTexture("DiffuseMap", solidWhite.GetID(), 0, SamplerType::Texture2D);

                for (auto& mesh : meshRenderer.m_Model->GetMeshes())
                    mesh.Render(shader);
                shader->Unbind();
            };

            auto sceneView = scene->GetAllEntitiesWith<MeshRendererComponent>();
            for (auto& entityID : sceneView) {
                Entity entity(entityID, scene.get());

                if (entity == graphicsContext->activeEntity) {
                    glEnable(GL_STENCIL_TEST);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                    glStencilMask(0x00);
                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
                    glStencilMask(0xFF);
                    DrawEntity(entity);
                    glDisable(GL_STENCIL_TEST);
                }
                DrawEntity(entity);
            };

            RenderLight(camera, graphicsContext, context);
            RenderOutline(camera, graphicsContext, context);

            glDisable(GL_CULL_FACE);
        }

        void Bind(const std::shared_ptr<Camera>            camera,
                  const std::shared_ptr<GraphicsContext>   graphicsContext,
                  const std::shared_ptr<PrecomputeContext> context);

        void RenderLight(const std::shared_ptr<Camera>            camera,
                         const std::shared_ptr<GraphicsContext>   graphicsContext,
                         const std::shared_ptr<PrecomputeContext> context);

        void RenderOutline(const std::shared_ptr<Camera>            camera,
                           const std::shared_ptr<GraphicsContext>   graphicsContext,
                           const std::shared_ptr<PrecomputeContext> context);

    private:
        std::shared_ptr<Shader>    m_GridShader    = std::make_shared<Shader>("line.vert", "line.frag");
        std::shared_ptr<Shader>    m_OutlineShader = std::make_shared<Shader>("common.vert", "outline.frag");
        std::shared_ptr<Shader>    m_IconShader    = std::make_shared<Shader>("quad.vert", "quad.frag");
        std::shared_ptr<Texture2D> m_LightIcon = std::make_shared<Texture2D>("../Assets/Icons/icon-light.png", TextureFormat::RGBA);
    };

    class OutlineRenderPass : public RenderPass {
    public:
        OutlineRenderPass() { m_OutlineShader = std::make_shared<Shader>("common.vert", "outline.frag"); }
        virtual void Render(const std::shared_ptr<Camera>            camera,
                            const std::shared_ptr<Scene>             scene,
                            const std::shared_ptr<GraphicsContext>   graphicsContext,
                            const std::shared_ptr<PrecomputeContext> context) override
        {
            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);

            // render container
            auto view = camera->GetView();
            auto proj = camera->GetProjection();

            m_OutlineShader->Bind();
            if (auto entity = graphicsContext->activeEntity) {
                // if (m_Running) object->OnUpdate(0.03f);
                auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();

                m_OutlineShader->SetInt("entityID", static_cast<int>(entity.GetID()));

                // MVP & Light MVP
                auto transform    = entity.GetComponent<TransformComponent>();
                transform.m_Scale = vec3(1.01);
                m_OutlineShader->SetMaterix4("model", glm::value_ptr(transform.GetTransform()));
                m_OutlineShader->SetMaterix4("view", glm::value_ptr(view));
                m_OutlineShader->SetMaterix4("proj", glm::value_ptr(proj));

                for (auto& mesh : meshRenderer.m_Model->GetMeshes())
                    mesh.Render(m_OutlineShader);
            }
            m_OutlineShader->Unbind();

            glStencilMask(0xFF);
            glDisable(GL_STENCIL_TEST);
        }

    private:
        std::shared_ptr<Shader> m_OutlineShader;
    };
}  // namespace suplex