#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/Renderer2D.h"

namespace Titan
{

    Scene::Scene() {}

    Scene::~Scene() {}

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = {m_Registry.create(), this};
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }

    void Scene::OnUpdate(Timestep ts)
    {
        TI_PROFILE_FUNCTION()
        Camera* mainCamera = nullptr;
        glm::mat4* cameraTransform = nullptr;
        {
            TI_PROFILE_SCOPE("Scene::OnUpdate Find Camera")
            auto view = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : view)
            {
                auto& transform = view.get<TransformComponent>(entity);
                auto& camera = view.get<CameraComponent>(entity);
                if (camera.Primary)
                {
                    mainCamera = &camera.Camera;
                    cameraTransform = &transform.Transform;
                    break;
                }
            }
        }

        if (mainCamera)
        {
            TI_PROFILE_SCOPE("Scene::OnUpdate Render")
            Renderer2D::BeginScene(mainCamera->GetProjectionMatrix(), *cameraTransform);

            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group)
            {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

                Renderer2D::DrawTransformedQuad(transform, sprite.Color);
            }

            Renderer2D::EndScene();
        }
    }

} // namespace Titan