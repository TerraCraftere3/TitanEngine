#include "SceneRenderer.h"
#include "PostProcessing.h"
#include "PostProcessing/FXAA.h"
#include "PostProcessing/Tonemapping.h"
#include "RenderGraph.h"
#include "Titan/Renderer/GeometryRenderer.h"
#include "Titan/Renderer/PBRRenderer.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Renderer/Renderer2D.h"
#include "Titan/Renderer/SkyboxRenderer.h"
#include "Titan/Scene/Components.h"
#include "Titan/Scene/Scene.h"

namespace Titan
{
    struct SceneViewData
    {
        Ref<RenderGraph> renderGraph;
        Ref<Framebuffer> finalFramebuffer;
        Ref<PostProcessingStack> postFXs;

        // Camera data (shared across passes)
        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
        glm::mat4 viewProjection{1.0f};
        glm::vec3 viewPosition{0.0f};
        uint32_t viewWidth = 1280;
        uint32_t viewHeight = 720;

        bool drawOverlay = false;
        bool drawAABBOVerlay = false;
        bool drawWireframe = false;

        Ref<Scene> currentScene;
    };

    struct SceneRendererData
    {
        std::array<SceneViewData, SceneRenderer::kMaxViews> views;
    };

    SceneRendererData* s_SRData = nullptr;

    void SceneRenderer::Init()
    {
        s_SRData = new SceneRendererData();
        // Initialize all views
        for (uint32_t i = 0; i < kMaxViews; ++i)
        {
            EnsureView(i);
        }
    }

    void SceneRenderer::EnsureView(uint32_t viewIndex)
    {
        TI_ASSERT(viewIndex < kMaxViews);
        TI_CORE_INFO("Setting up Scene Renderer View {}", viewIndex);
        auto& v = s_SRData->views[viewIndex];

        // Create final output framebuffer per view
        if (!v.finalFramebuffer)
        {
            FramebufferSpecification fbSpec;
            fbSpec.Attachments = {FramebufferTextureFormat::RGB16F, FramebufferTextureFormat::RED_INTEGER,
                                  FramebufferTextureFormat::Depth};
            fbSpec.Width = v.viewWidth;
            fbSpec.Height = v.viewHeight;
            v.finalFramebuffer = Framebuffer::Create(fbSpec);
        }

        // Post FX stack per view
        if (!v.postFXs)
        {
            v.postFXs = CreateRef<PostProcessingStack>();
            v.postFXs->AddEffect(CreateRef<TonemappingEffect>());
            v.postFXs->AddEffect(CreateRef<FXAAEffect>());
        }

        // Build/update the render graph for this view
        SetupRenderGraph(viewIndex);
    }

    void SceneRenderer::SetupRenderGraph(uint32_t viewIndex)
    {
        auto& view = s_SRData->views[viewIndex];
        if (!view.renderGraph)
            view.renderGraph = CreateRef<RenderGraph>();

        RenderGraphBuilder builder(*view.renderGraph);

        // Define resources
        builder
            .CreateFramebuffer("SceneFramebuffer",
                               {
                                   FramebufferTextureFormat::RGB16F,      // SceneColor (HDR)
                                   FramebufferTextureFormat::RED_INTEGER, // EntityID
                                   FramebufferTextureFormat::Depth        // SceneDepth
                               },
                               view.viewWidth, view.viewHeight)
            .CreateFramebuffer("GeometryBuffer",
                               {
                                   FramebufferTextureFormat::RGBA16F,     // Position
                                   FramebufferTextureFormat::RGBA16F,     // Normal
                                   FramebufferTextureFormat::RGBA8,       // Albedo
                                   FramebufferTextureFormat::RGBA8,       // Metallic, Roughness, AO, -
                                   FramebufferTextureFormat::RGBA8,       // Emission
                                   FramebufferTextureFormat::RED_INTEGER, // EntityID
                                   FramebufferTextureFormat::Depth        // Depth
                               },
                               view.viewWidth, view.viewHeight)
            .CreateFramebuffer("PostTonemapping",
                               {
                                   FramebufferTextureFormat::RGB16F,      // SceneColor (HDR)
                                   FramebufferTextureFormat::RED_INTEGER, // EntityID
                               },
                               view.viewWidth, view.viewHeight)
            .CreateFramebuffer("PostFXAA",
                               {
                                   FramebufferTextureFormat::RGB16F,      // SceneColor (HDR)
                                   FramebufferTextureFormat::RED_INTEGER, // EntityID
                               },
                               view.viewWidth, view.viewHeight)
            .CreatePersistentTexture("PreFX", FramebufferTextureFormat::RGBA8, view.viewWidth, view.viewHeight)
            .CreatePersistentTexture("FinalOutput", FramebufferTextureFormat::RGB16F, view.viewWidth, view.viewHeight);

        // Capture the view pointer so each pass uses its own camera/flags
        SceneViewData* v = &view;

        builder.AddRenderPass("ClearPass", {}, {"SceneFramebuffer", "PreFX"},
                              [v](RenderGraph& graph, const RenderPass& pass)
                              {
                                  auto fb = graph.GetFramebuffer("SceneFramebuffer");
                                  if (!fb)
                                      return;
                                  RenderCommand::BeginRenderPass(fb, "Clear Pass (-)");
                                  fb->ClearAttachment(1, -1);
                                  RenderCommand::Clear(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
                                  RenderCommand::EndRenderPass();
                              });

        builder.AddRenderPass(
            "GeometryPass", {}, {"GeometryBuffer", "PreFX"},
            [v](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("GeometryBuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Geometry Pass (3D)");
                RenderCommand::Clear(glm::vec4(0.0));
                GeometryRenderer::BeginScene(v->viewProjection,
                                             v->drawWireframe ? PolygonMode::Line : PolygonMode::Fill);
                auto meshView = v->currentScene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
                for (auto entity : meshView)
                {
                    auto [transform, meshComp] = meshView.get<TransformComponent, MeshRendererComponent>(entity);
                    if (meshComp.MeshRef)
                        GeometryRenderer::DrawMesh(meshComp.MeshRef, transform.GetTransform(), (uint32_t)entity);
                }
                GeometryRenderer::EndScene();
                RenderCommand::EndRenderPass();
            });

        builder.AddRenderPass(
            "PBRPass", {"GeometryBuffer", "SceneFramebuffer", "PreFX"}, {"SceneFramebuffer"},
            [v](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                auto gbuffer = graph.GetFramebuffer("GeometryBuffer");
                if (!fb || !gbuffer)
                    return;
                RenderCommand::BeginRenderPass(fb, "PBR Pass (3D)");
                bool hasDirectionalLight = false;
                glm::vec3 lightDirection;
                auto dlView = v->currentScene->GetAllEntitiesWith<TransformComponent, DirectionalLightComponent>();
                for (auto entity : dlView)
                {
                    auto [transform, dlComp] = dlView.get<TransformComponent, DirectionalLightComponent>(entity);
                    hasDirectionalLight = true;
                    lightDirection = dlComp.Direction;
                    break;
                }
                PBRSceneData data;
                data.HasDirectionalLight = hasDirectionalLight;
                data.LightDirection = lightDirection;
                data.ViewPosition = v->viewPosition;
                Ref<Cubemap> cubemap = nullptr;
                auto skyboxView = v->currentScene->GetAllEntitiesWith<TransformComponent, SkyboxComponent>();
                for (auto entity : skyboxView)
                {
                    auto [transform, sb] = skyboxView.get<TransformComponent, SkyboxComponent>(entity);
                    if (sb.mode == SkyboxComponent::Mode::HDRI)
                        cubemap = sb.hdriSettings.Irradiance;
                    break;
                }
                PBRRenderer::Render(graph.GetFramebuffer("GeometryBuffer"), data, cubemap);
                RenderCommand::EndRenderPass();
            });

        builder.AddRenderPass(
            "SpritePass", {}, {"SceneFramebuffer", "PreFX"},
            [v](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Sprite Pass (2D)");
                Renderer2D::BeginScene(v->viewProjection);
                auto spriteView = v->currentScene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();
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
                RenderCommand::EndRenderPass();
            });

        builder.AddRenderPass(
            "CirclePass", {}, {"SceneFramebuffer", "PreFX"},
            [v](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Circle Pass (2D)");
                Renderer2D::BeginScene(v->viewProjection);
                auto circleView = v->currentScene->GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();
                for (auto entity : circleView)
                {
                    auto [transform, circle] = circleView.get<TransformComponent, CircleRendererComponent>(entity);
                    Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade,
                                           (uint32_t)entity);
                }
                Renderer2D::EndScene();
                RenderCommand::EndRenderPass();
            });

        builder.AddRenderPass(
            "SkyboxPass", {}, {"SceneFramebuffer", "PreFX"},
            [v](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Skybox Pass (3D)");
                auto skyboxView = v->currentScene->GetAllEntitiesWith<TransformComponent, SkyboxComponent>();
                for (auto entity : skyboxView)
                {
                    auto [transform, sb] = skyboxView.get<TransformComponent, SkyboxComponent>(entity);
                    switch (sb.mode)
                    {
                        case SkyboxComponent::Mode::HDRI:
                        {
                            Ref<Cubemap> cubemap = sb.hdriSettings.Skybox;
                            if (cubemap)
                                SkyboxRenderer::Render(cubemap, v->view, v->projection);
                            break;
                        }
                        case SkyboxComponent::Mode::Colorramp:
                        {
                            SkyboxRenderer::Render(sb.colorrampSettings.TopColor, sb.colorrampSettings.BottomColor,
                                                   v->view, v->projection);
                            break;
                        }
                    }
                    break;
                }
                RenderCommand::EndRenderPass();
            });

        builder.AddRenderPass(
            "OverlayPass", {}, {"SceneFramebuffer", "PreFX"},
            [v](RenderGraph& graph, const RenderPass& pass)
            {
                if (!v->drawOverlay)
                    return;
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Overlay Pass (2D + 3D)");
                Renderer2D::BeginScene(v->viewProjection);
                auto boxColliderView =
                    v->currentScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                for (auto entity : boxColliderView)
                {
                    auto [transform, collider] =
                        boxColliderView.get<TransformComponent, BoxCollider2DComponent>(entity);
                    glm::mat4 world = transform.GetTransform();
                    glm::mat4 local = glm::translate(glm::mat4(1.0f), glm::vec3(collider.Offset, 0.0f)) *
                                      glm::scale(glm::mat4(1.0f), glm::vec3(collider.Size * 2.0f, 1.0f));
                    glm::mat4 finalTransform = world * local;
                    Renderer2D::DrawRect(finalTransform, glm::vec4(0.0f, 0.9f, 0.0f, 1.0));
                }
                auto circleColliderView =
                    v->currentScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                for (auto entity : circleColliderView)
                {
                    auto [transform, collider] =
                        circleColliderView.get<TransformComponent, CircleCollider2DComponent>(entity);
                    glm::mat4 world = transform.GetTransform();
                    glm::mat4 local =
                        glm::translate(glm::mat4(1.0f), glm::vec3(collider.Offset, 0.0f)) *
                        glm::scale(glm::mat4(1.0f), glm::vec3(collider.Radius * 2.0f, collider.Radius * 2.0f, 1.0f));
                    glm::mat4 finalTransform = world * local;
                    Renderer2D::DrawCircle(finalTransform, glm::vec4(0.0f, 0.9f, 0.0f, 1.0f));
                }
                auto cubeColliderView =
                    v->currentScene->GetAllEntitiesWith<TransformComponent, CubeColliderComponent>();
                for (auto entity : cubeColliderView)
                {
                    auto [transform, collider] =
                        cubeColliderView.get<TransformComponent, CubeColliderComponent>(entity);
                    glm::mat4 world = transform.GetTransform();
                    glm::mat4 local = glm::translate(glm::mat4(1.0f), collider.Offset) *
                                      glm::scale(glm::mat4(1.0f), collider.Size * 2.0f);
                    glm::mat4 finalTransform = world * local;
                    Renderer2D::DrawCube(finalTransform, glm::vec4(0.0f, 0.9f, 0.0f, 1.0f));
                }
                auto cameraView = v->currentScene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
                for (auto entity : cameraView)
                {
                    auto [transform, cc] = cameraView.get<TransformComponent, CameraComponent>(entity);
                    Renderer2D::DrawCamera(transform.GetTransform(), (uint32_t)entity);
                }
                auto lookAtView = v->currentScene->GetAllEntitiesWith<TransformComponent, LookAtComponent>();
                for (auto e : lookAtView)
                {
                    auto& transform = lookAtView.get<TransformComponent>(e);
                    auto& lookAt = lookAtView.get<LookAtComponent>(e);
                    glm::mat4 gizmoTransformation = glm::translate(glm::mat4(1.0f), lookAt.Position);
                    Renderer2D::DrawMarker(gizmoTransformation);
                }
                if (v->drawAABBOVerlay)
                {
                    auto meshView = v->currentScene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
                    for (auto entity : meshView)
                    {
                        auto [transform, meshComp] = meshView.get<TransformComponent, MeshRendererComponent>(entity);
                        if (meshComp.MeshRef)
                        {
                            AABB bounds = meshComp.MeshRef->GetBounds();
                            glm::mat4 world = transform.GetTransform();
                            glm::vec3 center = (bounds.Min + bounds.Max) * 0.5f;
                            glm::vec3 size = (bounds.Max - bounds.Min);
                            glm::mat4 local =
                                glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);
                            glm::mat4 finalTransform = world * local;
                            glm::vec4 color = {1.0f, 0.0f, 0.0f, 1.0f};
                            Renderer2D::DrawCube(finalTransform, color);
                        }
                    }
                }
                Renderer2D::DrawGrid(20.0f);
                Renderer2D::EndScene();
                RenderCommand::EndRenderPass();
            });

        builder.AddRenderPass("Tonemapping", {"SceneFramebuffer", "GeometryBuffer", "PreFX"}, {"PostTonemapping"},
                              [v](RenderGraph& graph, const RenderPass& pass)
                              {
                                  auto inputFB = graph.GetFramebuffer("SceneFramebuffer");
                                  auto gbuffer = graph.GetFramebuffer("GeometryBuffer");
                                  auto outputFB = graph.GetFramebuffer("PostTonemapping");
                                  if (!inputFB || !outputFB || !gbuffer)
                                      return;

                                  PostFXComponent tonemappingFX;
                                  auto tonemappingView = v->currentScene->GetAllEntitiesWith<PostFXComponent>();
                                  for (auto entity : tonemappingView)
                                  {
                                      tonemappingFX = tonemappingView.get<PostFXComponent>(entity);
                                      break;
                                  }
                                  PostFXInput input{graph,           pass,         inputFB, outputFB, gbuffer,
                                                    v->currentScene, tonemappingFX};
                                  v->postFXs->GetEffect<TonemappingEffect>()->Render(input);
                              });

        builder.AddRenderPass("FXAA", {"SceneFramebuffer", "GeometryBuffer", "PostTonemapping"}, {"PostFXAA"},
                              [v](RenderGraph& graph, const RenderPass& pass)
                              {
                                  auto inputFB = graph.GetFramebuffer("PostTonemapping");
                                  auto gbuffer = graph.GetFramebuffer("GeometryBuffer");
                                  auto outputFB = graph.GetFramebuffer("PostFXAA");
                                  if (!inputFB || !outputFB || !gbuffer)
                                      return;

                                  PostFXComponent fxaaFX;
                                  auto fxaaView = v->currentScene->GetAllEntitiesWith<PostFXComponent>();
                                  for (auto entity : fxaaView)
                                  {
                                      fxaaFX = fxaaView.get<PostFXComponent>(entity);
                                      break;
                                  }
                                  PostFXInput input{graph, pass, inputFB, outputFB, gbuffer, v->currentScene, fxaaFX};
                                  v->postFXs->GetEffect<FXAAEffect>()->Render(input);
                              });

        // Build the graph
        builder.Build();

#if TI_BUILD_DEBUG
        view.renderGraph->ExportToDOT("scene-graph.generated." + std::to_string(viewIndex) + ".dot");
#endif
    }

    void SceneRenderer::Shutdown()
    {
        if (s_SRData)
        {
            for (uint32_t i = 0; i < kMaxViews; ++i)
            {
                TI_CORE_INFO("Shutting down Scene Renderer View {}", i);
                auto& v = s_SRData->views[i];
                if (v.postFXs)
                    v.postFXs->Shutdown();
            }
            delete s_SRData;
            s_SRData = nullptr;
        }
    }

    void SceneRenderer::RenderSceneRuntime(Ref<Scene> scene)
    {
        RenderSceneRuntime(0, scene);
    }

    void SceneRenderer::RenderSceneRuntime(uint32_t viewIndex, Ref<Scene> scene)
    {
        TI_ASSERT(viewIndex < kMaxViews);
        auto& v = s_SRData->views[viewIndex];

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
            v.view = glm::inverse(cameraTransform);
            v.projection = mainCamera->GetProjection();
            v.viewProjection = mainCamera->GetProjection() * glm::inverse(cameraTransform);
            v.viewPosition = glm::vec3(cameraTransform[3]);
            v.drawOverlay = false;
            v.currentScene = scene;

            if (v.renderGraph)
                v.renderGraph->Execute();
        }
    }

    void SceneRenderer::RenderSceneEditor(Ref<Scene> scene, EditorCamera& camera, OverlaySettings overlay)
    {
        RenderSceneEditor(0, scene, camera, overlay);
    }

    void SceneRenderer::RenderSceneEditor(uint32_t viewIndex, Ref<Scene> scene, EditorCamera& camera,
                                          OverlaySettings overlay)
    {
        TI_ASSERT(viewIndex < kMaxViews);
        auto& v = s_SRData->views[viewIndex];
        v.view = camera.GetViewMatrix();
        v.projection = camera.GetProjectionMatrix();
        v.viewProjection = camera.GetViewProjection();
        v.viewPosition = camera.GetPosition();
        v.drawOverlay = overlay.enableOverlay;
        v.drawAABBOVerlay = overlay.enableBoundingBoxRender;
        v.drawWireframe = overlay.enableWireframe;
        v.currentScene = scene;

        if (v.renderGraph)
            v.renderGraph->Execute();
    }

    void SceneRenderer::Resize(uint32_t width, uint32_t height)
    {
        Resize(0, width, height);
    }

    void SceneRenderer::Resize(uint32_t viewIndex, uint32_t width, uint32_t height)
    {
        TI_ASSERT(viewIndex < kMaxViews);
        if (width == 0 || height == 0)
            return;

        auto& v = s_SRData->views[viewIndex];
        if (v.viewWidth == width && v.viewHeight == height)
            return;

        v.viewWidth = width;
        v.viewHeight = height;

        if (v.finalFramebuffer)
            v.finalFramebuffer->Resize(width, height);

        if (v.renderGraph)
        {
            for (auto& [name, fb] : v.renderGraph->GetFramebuffers())
            {
                if (fb)
                    fb->Resize(width, height);
            }
        }
    }

    Ref<Framebuffer> SceneRenderer::GetFramebuffer()
    {
        return GetFramebuffer(0);
    }

    Ref<Framebuffer> SceneRenderer::GetFramebuffer(uint32_t viewIndex)
    {
        TI_ASSERT(viewIndex < kMaxViews);
        auto& v = s_SRData->views[viewIndex];
        auto finalFB = v.renderGraph ? v.renderGraph->GetFramebuffer("PostFXAA") : nullptr;
        return finalFB ? finalFB : v.finalFramebuffer;
    }

} // namespace Titan