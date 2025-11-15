#include "SceneRenderer.h"
#include "RenderGraph.h"
#include "Titan/Renderer/PBRRenderer.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Renderer/Renderer2D.h"
#include "Titan/Renderer/Renderer3D.h"
#include "Titan/Scene/Components.h"
#include "Titan/Scene/Scene.h"

namespace Titan
{
    struct SceneRendererData
    {
        Ref<RenderGraph> renderGraph;
        Ref<Framebuffer> finalFramebuffer;

        // Camera data (shared across passes)
        glm::mat4 viewProjection;
        glm::vec3 viewPosition;
        uint32_t viewWidth = 1280;
        uint32_t viewHeight = 720;

        bool drawOverlay = false;

        Ref<Scene> currentScene;
    };

    SceneRendererData* s_SRData = nullptr;

    void SceneRenderer::Init()
    {
        s_SRData = new SceneRendererData();
        s_SRData->renderGraph = CreateRef<RenderGraph>();

        // Create final output framebuffer
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = {FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER,
                              FramebufferTextureFormat::Depth};
        fbSpec.Width = s_SRData->viewWidth;
        fbSpec.Height = s_SRData->viewHeight;
        fbSpec.Samples = 1;
        s_SRData->finalFramebuffer = Framebuffer::Create(fbSpec);

        SetupRenderGraph();
    }

    void SceneRenderer::SetupRenderGraph()
    {
        auto& graph = *s_SRData->renderGraph;
        RenderGraphBuilder builder(graph);

        // Define resources
        builder
            .CreateFramebuffer("SceneFramebuffer",
                               {
                                   FramebufferTextureFormat::RGBA8,       // SceneColor
                                   FramebufferTextureFormat::RED_INTEGER, // EntityID
                                   FramebufferTextureFormat::Depth        // SceneDepth
                               },
                               s_SRData->viewWidth, s_SRData->viewHeight, 1)
            .CreateFramebuffer("GeometryBuffer",
                               {
                                   FramebufferTextureFormat::RGBA16F,     // Position
                                   FramebufferTextureFormat::RGBA16F,     // Normal
                                   FramebufferTextureFormat::RGBA8,       // Albedo
                                   FramebufferTextureFormat::RGBA8,       // Metallic, Roughness, AO, -
                                   FramebufferTextureFormat::RED_INTEGER, // EntityID
                                   FramebufferTextureFormat::Depth        // Depth
                               },
                               s_SRData->viewWidth, s_SRData->viewHeight, 1)
            .CreatePersistentTexture("FinalOutput", FramebufferTextureFormat::RGBA8, s_SRData->viewWidth,
                                     s_SRData->viewHeight, 1);

        builder.AddRenderPass(
            "ClearPass", {}, {"SceneFramebuffer"},
            [](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;

                fb->Bind();
                fb->ClearAttachment(1, -1); // Clear entity ID (assuming index 1 = RED_INTEGER)
                RenderCommand::SetClearColor({173.0f / 255.0f, 216.0f / 255.0f, 230.0f / 255.0f, 1.0f});
                RenderCommand::Clear();
                fb->Unbind();
            });

        builder.AddRenderPass(
            "GeometryPass", {}, {"GeometryBuffer"},
            [](RenderGraph& graph, const RenderPass& pass)

            {
                auto fb = graph.GetFramebuffer("GeometryBuffer");

                auto meshView = s_SRData->currentScene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
                bool hasMeshes = meshView.begin() != meshView.end();

                if (!fb)
                    return;

                fb->Bind();

                RenderCommand::SetClearColor(glm::vec4(0.0));
                RenderCommand::Clear();
                Renderer3D::BeginScene(s_SRData->viewProjection);

                for (auto entity : meshView)
                {
                    auto [transform, meshComp] = meshView.get<TransformComponent, MeshRendererComponent>(entity);
                    if (meshComp.MeshRef)
                        Renderer3D::DrawMesh(meshComp.MeshRef, transform.GetTransform(), (uint32_t)entity);
                }

                Renderer3D::EndScene();
                fb->Unbind();
            });

        builder.AddRenderPass(
            "PBRPass", {"GeometryBuffer", "SceneFramebuffer"}, {"SceneFramebuffer"},
            [](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                auto gbuffer = graph.GetFramebuffer("GeometryBuffer");

                auto meshView = s_SRData->currentScene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
                bool hasMeshes = meshView.begin() != meshView.end();

                if (!fb || !gbuffer)
                    return;

                fb->Bind();

                bool hasDirectionalLight = false;
                glm::vec3 lightDirection;
                auto dlView =
                    s_SRData->currentScene->GetAllEntitiesWith<TransformComponent, DirectionalLightComponent>();
                for (auto entity : dlView)
                {
                    auto [transform, dlComp] = dlView.get<TransformComponent, DirectionalLightComponent>(entity);
                    hasDirectionalLight = true;
                    lightDirection = dlComp.Direction;

                    break; // only use first
                }

                PBRSceneData data;
                data.HasDirectionalLight = hasDirectionalLight;
                data.LightDirection = lightDirection;
                data.ViewPosition = s_SRData->viewPosition;

                PBRRenderer::Render(graph.GetFramebuffer("GeometryBuffer"), data);

                fb->Unbind();
            });

        builder.AddRenderPass(
            "SpritePass", {}, {"SceneFramebuffer"},
            [](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;

                fb->Bind();

                Renderer2D::BeginScene(s_SRData->viewProjection);

                auto spriteView =
                    s_SRData->currentScene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();
                for (auto entity : spriteView)
                {
                    auto [transform, sprite] = spriteView.get<TransformComponent, SpriteRendererComponent>(entity);

                    if (sprite.Tex)
                        Renderer2D::DrawTransformedQuad(transform.GetTransform(), sprite.Tex, 1.0f, sprite.Color,
                                                        (uint32_t)entity);
                    else
                        Renderer2D::DrawTransformedQuad(transform.GetTransform(), sprite.Color, (uint32_t)entity);
                }

                Renderer2D::EndScene();
                fb->Unbind();
            });

        builder.AddRenderPass(
            "CirclePass", {}, {"SceneFramebuffer"},
            [](RenderGraph& graph, const RenderPass& pass)

            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;

                fb->Bind();

                Renderer2D::BeginScene(s_SRData->viewProjection);

                auto circleView =
                    s_SRData->currentScene->GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();
                for (auto entity : circleView)
                {
                    auto [transform, circle] = circleView.get<TransformComponent, CircleRendererComponent>(entity);
                    Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade,
                                           (uint32_t)entity);
                }

                Renderer2D::EndScene();
                fb->Unbind();
            });

        builder.AddRenderPass(
            "OverlayPass", {}, {"SceneFramebuffer"},
            [](RenderGraph& graph, const RenderPass& pass)

            {
                if (!s_SRData->drawOverlay)
                    return;

                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;

                fb->Bind();

                Renderer2D::BeginScene(s_SRData->viewProjection);

                // Box Colliders
                auto boxColliderView =
                    s_SRData->currentScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                for (auto entity : boxColliderView)
                {
                    auto [transform, collider] =
                        boxColliderView.get<TransformComponent, BoxCollider2DComponent>(entity);
                    Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(0.0f, 0.9f, 0.0f, 1.0));
                }

                // Circle Colliders
                auto circleColliderView =
                    s_SRData->currentScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                for (auto entity : circleColliderView)
                {
                    auto [transform, collider] =
                        circleColliderView.get<TransformComponent, CircleCollider2DComponent>(entity);
                    Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(0.0f, 0.9f, 0.0f, 1.0));
                }

                auto cameraView = s_SRData->currentScene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
                for (auto entity : cameraView)
                {
                    auto [transform, cc] = cameraView.get<TransformComponent, CameraComponent>(entity);
                    Renderer2D::DrawCamera(transform.GetTransform());
                }

                Renderer2D::DrawGrid(20.0f);

                Renderer2D::EndScene();
                fb->Unbind();
            });

        builder.AddRenderPass("ResolvePass", {"SceneFramebuffer"}, {"FinalOutput"},
                              [](RenderGraph& graph, const RenderPass& pass)
                              {
                                  auto sceneFB = graph.GetFramebuffer("SceneFramebuffer");
                                  if (sceneFB)
                                      sceneFB->Resolve();
                              });

        // Build the graph
        builder.Build();
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

        if (mainCamera)
        {
            s_SRData->viewProjection = mainCamera->GetProjection() * glm::inverse(cameraTransform);
            s_SRData->viewPosition = glm::vec3(cameraTransform[3]);
            s_SRData->drawOverlay = false;
            s_SRData->currentScene = scene;

            s_SRData->renderGraph->Execute();
        }
    }

    void SceneRenderer::RenderSceneEditor(Ref<Scene> scene, EditorCamera& camera)
    {
        s_SRData->viewProjection = camera.GetViewProjection();
        s_SRData->viewPosition = camera.GetPosition();
        s_SRData->drawOverlay = true;
        s_SRData->currentScene = scene;

        s_SRData->renderGraph->Execute();
    }

    void SceneRenderer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        if (s_SRData->viewWidth == width && s_SRData->viewHeight == height)
            return;

        s_SRData->viewWidth = width;
        s_SRData->viewHeight = height;

        if (s_SRData->finalFramebuffer)
            s_SRData->finalFramebuffer->Resize(width, height);

        auto& graph = *s_SRData->renderGraph;

        for (auto& [name, fb] : graph.GetFramebuffers())
        {
            if (fb)
                fb->Resize(width, height);
        }
    }

    Ref<Framebuffer> SceneRenderer::GetFramebuffer()
    {
        // Return the final framebuffer that contains the rendered scene
        auto finalFB = s_SRData->renderGraph->GetFramebuffer("SceneFramebuffer");
        return finalFB ? finalFB : s_SRData->finalFramebuffer;
    }

} // namespace Titan