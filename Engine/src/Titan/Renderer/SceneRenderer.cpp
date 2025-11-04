#include "SceneRenderer.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Renderer/Renderer2D.h"
#include "Titan/Renderer/Renderer3D.h"
#include "Titan/Scene/Components.h"
#include "Titan/Scene/Scene.h"

namespace Titan
{
    struct SceneRendererData
    {
        Ref<Framebuffer> framebuffer;
    };

    SceneRendererData* s_SRData = nullptr;

    void SceneRenderer::Init()
    {
        s_SRData = new SceneRendererData();

        // Framebuffer
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = {FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER,
                              FramebufferTextureFormat::Depth};
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.Samples = 1;
        s_SRData->framebuffer = Framebuffer::Create(fbSpec);
    }

    void SceneRenderer::Shutdown()
    {
        delete s_SRData;
        s_SRData = nullptr;
    }

    void SceneRenderer::RenderSceneRuntime(Ref<Scene> scene)
    {
        Camera* mainCamera = nullptr;
        glm::mat4 cameraTransform;
        {
            auto view = scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
            for (auto entity : view)
            {
                auto& transform = view.get<TransformComponent>(entity);
                auto& camera = view.get<CameraComponent>(entity);
                if (camera.Primary)
                {
                    mainCamera = &camera.Camera;
                    cameraTransform = transform.GetTransform();
                    break;
                }
            }
        }

        // RENDER
        if (mainCamera)
        {
            RenderScene(scene, mainCamera->GetProjection() * glm::inverse(cameraTransform),
                        glm::vec3(cameraTransform[3]), false);
        }
    }

    void SceneRenderer::RenderSceneEditor(Ref<Scene> scene, EditorCamera& camera)
    {
        RenderScene(scene, camera.GetViewProjection(), camera.GetPosition(), true);
    }

    Ref<Framebuffer> SceneRenderer::GetFramebuffer()
    {
        return s_SRData->framebuffer;
    }

    void SceneRenderer::RenderScene(Ref<Scene> scene, const glm::mat4& viewTransform, const glm::vec3& viewPosition,
                                    bool drawOverlay)
    {
        TI_PROFILE_FUNCTION();

        s_SRData->framebuffer->Bind();
        s_SRData->framebuffer->ClearAttachment(1, -1);

        RenderCommand::SetClearColor({173.0f / 255.0f, 216.0f / 255.0f, 230.0f / 255.0f, 1.0f});
        RenderCommand::Clear();

        Renderer2D::BeginScene(viewTransform);

        {
            auto spriteView = scene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();
            for (auto entity : spriteView)
            {
                auto [transform, sprite] = spriteView.get<TransformComponent, SpriteRendererComponent>(entity);

                if (sprite.Tex)
                    Renderer2D::DrawTransformedQuad(transform.GetTransform(), sprite.Tex, 1.0f, sprite.Color,
                                                    (uint32_t)entity);
                else
                    Renderer2D::DrawTransformedQuad(transform.GetTransform(), sprite.Color, (uint32_t)entity);
            }
        }
        {
            auto circleView = scene->GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();
            for (auto entity : circleView)
            {
                auto [transform, circle] = circleView.get<TransformComponent, CircleRendererComponent>(entity);

                Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade,
                                       (uint32_t)entity);
            }
        }
        if (drawOverlay)
        {
            {
                auto colliderView = scene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                for (auto entity : colliderView)
                {
                    auto [transform, collider] = colliderView.get<TransformComponent, BoxCollider2DComponent>(entity);

                    Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(0.0f, 0.9f, 0.0f, 1.0));
                }
            }
            {
                auto colliderView = scene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                for (auto entity : colliderView)
                {
                    auto [transform, collider] =
                        colliderView.get<TransformComponent, CircleCollider2DComponent>(entity);

                    // Renderer2D::DrawCircle(transform.GetTransform(), glm::vec4(0, 1, 0, 1), 0.05f);
                    Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(0.0f, 0.9f, 0.0f, 1.0));
                }
            }
        }
        Renderer2D::EndScene();
        Renderer3D::BeginScene(viewTransform, viewPosition);
        {
            auto meshView = scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
            for (auto entity : meshView)
            {
                auto [transform, meshComp] = meshView.get<TransformComponent, MeshRendererComponent>(entity);
                if (meshComp.MeshRef)
                    Renderer3D::DrawMesh(meshComp.MeshRef, meshComp.Material, transform.GetTransform(),
                                         (uint32_t)entity);
            }
        }
        Renderer3D::EndScene();

        s_SRData->framebuffer->Unbind();
        s_SRData->framebuffer->Resolve();
    }
} // namespace Titan