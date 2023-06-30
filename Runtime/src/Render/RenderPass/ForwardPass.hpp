#pragma once
#include "Render/Config/Config.hpp"
#include "Render/Geometry/Mesh.hpp"
#include "Render/Geometry/Shape/Shape.hpp"
#include "Render/Shader/Shader.hpp"
#include "Render/Texture/Texture2D.hpp"
#include "RenderPass.hpp"
#include "Scene/Component/Component.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <Scene/Scene.hpp>
#include <vector>
#include <Scene/Entity/Entity.hpp>

extern suplex::Texture2D solidWhite;

namespace suplex {

    class ForwardRenderPass : public RenderPass {
    public:
        ForwardRenderPass() { m_GridShader = std::make_shared<Shader>("line.vert", "line.frag"); }

        virtual void Render(const std::shared_ptr<Camera>&            camera,
                            const std::shared_ptr<Scene>&             scene,
                            const std::shared_ptr<GraphicsConfig>&    config,
                            const std::shared_ptr<PrecomputeContext>& context) override
        {
            // render to custom framebuffer
            // ------
            glEnable(GL_CULL_FACE);
            glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
            // glClearColor(.6f, .7f, .9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            auto view = camera->GetView();
            auto proj = camera->GetProjection();

            auto& lightCamera = config->lightSetting.cameraLS;
            auto  viewLS      = lightCamera->GetView();
            auto  projLS      = lightCamera->GetOrthoProjection();

            auto mvpLS = projLS * viewLS;

            m_GridShader->Bind();
            m_GridShader->SetMaterix4("view", glm::value_ptr(view));
            m_GridShader->SetMaterix4("proj", glm::value_ptr(proj));
            utils::RenderGird(nullptr);
            m_GridShader->Unbind();

            // render container
            std::vector<Entity> entities;
            for (auto e : scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>())
                entities.push_back(Entity{e, scene.get()});

            for (auto& entity : entities) {
                // if (m_Running) object->OnUpdate(0.03f);

                auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
                auto& shader       = m_Shaders[meshRenderer.m_Model->GetMaterialIndex()];

                shader->Bind();
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
                shader->SetFloat3("baseColor", glm::value_ptr(config->pbrSetting.baseColor));
                shader->SetFloat("metallic", &config->pbrSetting.metallic);
                shader->SetFloat("roughness", &config->pbrSetting.roughness);
                shader->SetFloat("ao", &config->pbrSetting.ao);

                shader->BindTexture("DiffuseMap", solidWhite.GetID(), 0, SamplerType::Texture2D);

                for (auto& mesh : meshRenderer.m_Model->GetMeshes()) mesh.Render(shader);
                shader->Unbind();
            }

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
                lightShader->SetMaterix4("mvpLS", glm::value_ptr(mvpLS));

                utils::RenderSphere(lightShader);
                lightShader->Unbind();
            }

            // Return to default framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glDisable(GL_CULL_FACE);
        }

    private:
        std::shared_ptr<Shader> m_GridShader = nullptr;
    };
}  // namespace suplex