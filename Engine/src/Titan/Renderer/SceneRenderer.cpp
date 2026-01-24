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
    struct SceneRendererData
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

    Ref<SceneRenderer> SceneRenderer::Create()
    {
        return CreateRef<SceneRenderer>();
    }

    SceneRenderer::SceneRenderer()
    {
        m_Data = CreateScope<SceneRendererData>();
        EnsureResources();
    }

    SceneRenderer::~SceneRenderer()
    {
        if (m_Data && m_Data->postFXs)
            m_Data->postFXs->Shutdown();
    }

    void SceneRenderer::EnsureResources()
    {
        TI_CORE_INFO("Setting up Scene Renderer Instance");
        auto& d = *m_Data;

        // Create final output framebuffer
        if (!d.finalFramebuffer)
        {
            FramebufferSpecification fbSpec;
            fbSpec.Attachments = {FramebufferTextureFormat::RGB16F, FramebufferTextureFormat::RED_INTEGER,
                                  FramebufferTextureFormat::Depth};
            fbSpec.Width = d.viewWidth;
            fbSpec.Height = d.viewHeight;
            d.finalFramebuffer = Framebuffer::Create(fbSpec);
        }

        // Post FX stack
        if (!d.postFXs)
        {
            d.postFXs = CreateRef<PostProcessingStack>();
            d.postFXs->AddEffect(CreateRef<TonemappingEffect>());
            d.postFXs->AddEffect(CreateRef<FXAAEffect>());
        }

        // Build/update the render graph
        SetupRenderGraph();
    }

    void SceneRenderer::SetupRenderGraph()
    {
        auto& view = *m_Data;
        if (!m_Data->renderGraph)
            m_Data->renderGraph = CreateRef<RenderGraph>();

        RenderGraphBuilder builder(*m_Data->renderGraph);

        // Define resources
        builder
            .CreateFramebuffer("SceneFramebuffer",
                               {
                                   FramebufferTextureFormat::RGB16F,      // SceneColor (HDR)
                                   FramebufferTextureFormat::RED_INTEGER, // EntityID
                                   FramebufferTextureFormat::Depth        // SceneDepth
                               },
                               m_Data->viewWidth, m_Data->viewHeight)
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
                               m_Data->viewWidth, m_Data->viewHeight)
            .CreateFramebuffer("PostTonemapping",
                               {
                                   FramebufferTextureFormat::RGB16F,      // SceneColor (HDR)
                                   FramebufferTextureFormat::RED_INTEGER, // EntityID
                               },
                               m_Data->viewWidth, m_Data->viewHeight)
            .CreateFramebuffer("PostFXAA",
                               {
                                   FramebufferTextureFormat::RGB16F,      // SceneColor (HDR)
                                   FramebufferTextureFormat::RED_INTEGER, // EntityID
                               },
                               m_Data->viewWidth, m_Data->viewHeight)
            .CreatePersistentTexture("PreFX", FramebufferTextureFormat::RGBA8, m_Data->viewWidth, m_Data->viewHeight)
            .CreatePersistentTexture("FinalOutput", FramebufferTextureFormat::RGB16F, m_Data->viewWidth,
                                     m_Data->viewHeight);

        // Capture the view pointer so each pass uses its own camera/flags
        SceneRendererData* d = m_Data.get();

        builder.AddRenderPass("ClearPass", {}, {"SceneFramebuffer", "PreFX"},
                              [d](RenderGraph& graph, const RenderPass& pass)
                              {
                                  auto fb = graph.GetFramebuffer("SceneFramebuffer");
                                  if (!fb)
                                      return;
                                  RenderCommand::BeginRenderPass(fb, "Clear Pass (-)");
                                  RenderCommand::Clear(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
                                  fb->ClearAttachment(1, -1); // Clear EntityID attachment
                                  RenderCommand::EndRenderPass();
                              });

        builder.AddRenderPass(
            "GeometryPass", {}, {"GeometryBuffer", "PreFX"},
            [d](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("GeometryBuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Geometry Pass (3D)");
                RenderCommand::Clear(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
                GeometryRenderer::BeginScene(d->viewProjection,
                                             d->drawWireframe ? PolygonMode::Line : PolygonMode::Fill);
                auto meshView = d->currentScene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
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
            [d](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                auto gbuffer = graph.GetFramebuffer("GeometryBuffer");
                if (!fb || !gbuffer)
                    return;
                RenderCommand::BeginRenderPass(fb, "PBR Pass (3D)");
                bool hasDirectionalLight = false;
                glm::vec3 lightDirection;
                auto dlView = d->currentScene->GetAllEntitiesWith<TransformComponent, DirectionalLightComponent>();
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
                data.ViewPosition = d->viewPosition;
                Ref<Cubemap> cubemap = nullptr;
                auto skyboxView = d->currentScene->GetAllEntitiesWith<TransformComponent, SkyboxComponent>();
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
            [d](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Sprite Pass (2D)");
                Renderer2D::BeginScene(d->viewProjection);
                auto spriteView = d->currentScene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();
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
            [d](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Circle Pass (2D)");
                Renderer2D::BeginScene(d->viewProjection);
                auto circleView = d->currentScene->GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();
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
            [d](RenderGraph& graph, const RenderPass& pass)
            {
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Skybox Pass (3D)");
                auto skyboxView = d->currentScene->GetAllEntitiesWith<TransformComponent, SkyboxComponent>();
                for (auto entity : skyboxView)
                {
                    auto [transform, sb] = skyboxView.get<TransformComponent, SkyboxComponent>(entity);
                    switch (sb.mode)
                    {
                        case SkyboxComponent::Mode::HDRI:
                        {
                            Ref<Cubemap> cubemap = sb.hdriSettings.Skybox;
                            if (cubemap)
                                SkyboxRenderer::Render(cubemap, d->view, d->projection);
                            break;
                        }
                        case SkyboxComponent::Mode::Colorramp:
                        {
                            SkyboxRenderer::Render(sb.colorrampSettings.TopColor, sb.colorrampSettings.BottomColor,
                                                   d->view, d->projection);
                            break;
                        }
                    }
                    break;
                }
                RenderCommand::EndRenderPass();
            });

        builder.AddRenderPass(
            "OverlayPass", {}, {"SceneFramebuffer", "PreFX"},
            [d](RenderGraph& graph, const RenderPass& pass)
            {
                if (!d->drawOverlay)
                    return;
                auto fb = graph.GetFramebuffer("SceneFramebuffer");
                if (!fb)
                    return;
                RenderCommand::BeginRenderPass(fb, "Overlay Pass (2D + 3D)");
                Renderer2D::BeginScene(d->viewProjection);
                auto boxColliderView =
                    d->currentScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
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
                    d->currentScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
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
                    d->currentScene->GetAllEntitiesWith<TransformComponent, CubeColliderComponent>();
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
                auto cameraView = d->currentScene->GetAllEntitiesWith<TransformComponent, CameraComponent>();
                for (auto entity : cameraView)
                {
                    auto [transform, cc] = cameraView.get<TransformComponent, CameraComponent>(entity);
                    Renderer2D::DrawCamera(transform.GetTransform(), (uint32_t)entity);
                }
                auto lookAtView = d->currentScene->GetAllEntitiesWith<TransformComponent, LookAtComponent>();
                for (auto e : lookAtView)
                {
                    auto& transform = lookAtView.get<TransformComponent>(e);
                    auto& lookAt = lookAtView.get<LookAtComponent>(e);
                    glm::mat4 gizmoTransformation = glm::translate(glm::mat4(1.0f), lookAt.Position);
                    Renderer2D::DrawMarker(gizmoTransformation);
                }
                if (d->drawAABBOVerlay)
                {
                    auto meshView = d->currentScene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
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
                              [d](RenderGraph& graph, const RenderPass& pass)
                              {
                                  auto inputFB = graph.GetFramebuffer("SceneFramebuffer");
                                  auto gbuffer = graph.GetFramebuffer("GeometryBuffer");
                                  auto outputFB = graph.GetFramebuffer("PostTonemapping");
                                  if (!inputFB || !outputFB || !gbuffer)
                                      return;

                                  PostFXComponent tonemappingFX;
                                  auto tonemappingView = d->currentScene->GetAllEntitiesWith<PostFXComponent>();
                                  for (auto entity : tonemappingView)
                                  {
                                      tonemappingFX = tonemappingView.get<PostFXComponent>(entity);
                                      break;
                                  }
                                  PostFXInput input{graph,           pass,         inputFB, outputFB, gbuffer,
                                                    d->currentScene, tonemappingFX};
                                  d->postFXs->GetEffect<TonemappingEffect>()->Render(input);
                              });

        builder.AddRenderPass("FXAA", {"SceneFramebuffer", "GeometryBuffer", "PostTonemapping"}, {"PostFXAA"},
                              [d](RenderGraph& graph, const RenderPass& pass)
                              {
                                  auto inputFB = graph.GetFramebuffer("PostTonemapping");
                                  auto gbuffer = graph.GetFramebuffer("GeometryBuffer");
                                  auto outputFB = graph.GetFramebuffer("PostFXAA");
                                  if (!inputFB || !outputFB || !gbuffer)
                                      return;

                                  PostFXComponent fxaaFX;
                                  auto fxaaView = d->currentScene->GetAllEntitiesWith<PostFXComponent>();
                                  for (auto entity : fxaaView)
                                  {
                                      fxaaFX = fxaaView.get<PostFXComponent>(entity);
                                      break;
                                  }
                                  PostFXInput input{graph, pass, inputFB, outputFB, gbuffer, d->currentScene, fxaaFX};
                                  d->postFXs->GetEffect<FXAAEffect>()->Render(input);
                              });

        // Build the graph
        builder.Build();

#if TI_BUILD_DEBUG
        m_Data->renderGraph->ExportToDOT("scene-graph.generated.dot");
#endif
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
            m_Data->view = glm::inverse(cameraTransform);
            m_Data->projection = mainCamera->GetProjection();
            m_Data->viewProjection = mainCamera->GetProjection() * glm::inverse(cameraTransform);
            m_Data->viewPosition = glm::vec3(cameraTransform[3]);
            m_Data->drawOverlay = false;
            m_Data->currentScene = scene;

            if (m_Data->renderGraph)
                m_Data->renderGraph->Execute();
        }
    }

    void SceneRenderer::RenderSceneEditor(Ref<Scene> scene, EditorCamera& camera, OverlaySettings overlay)
    {
        m_Data->view = camera.GetViewMatrix();
        m_Data->projection = camera.GetProjectionMatrix();
        m_Data->viewProjection = camera.GetViewProjection();
        m_Data->viewPosition = camera.GetPosition();
        m_Data->drawOverlay = overlay.enableOverlay;
        m_Data->drawAABBOVerlay = overlay.enableBoundingBoxRender;
        m_Data->drawWireframe = overlay.enableWireframe;
        m_Data->currentScene = scene;

        if (m_Data->renderGraph)
            m_Data->renderGraph->Execute();
    }

    void SceneRenderer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0)
            return;

        if (m_Data->viewWidth == width && m_Data->viewHeight == height)
            return;

        m_Data->viewWidth = width;
        m_Data->viewHeight = height;

        if (m_Data->finalFramebuffer)
            m_Data->finalFramebuffer->Resize(width, height);

        if (m_Data->renderGraph)
        {
            for (auto& [name, fb] : m_Data->renderGraph->GetFramebuffers())
            {
                if (fb)
                    fb->Resize(width, height);
            }
        }
    }

    Ref<Framebuffer> SceneRenderer::GetFramebuffer()
    {
        auto& d = *m_Data;
        auto finalFB = d.renderGraph ? d.renderGraph->GetFramebuffer("PostFXAA") : nullptr;
        return finalFB ? finalFB : d.finalFramebuffer;
    }

} // namespace Titan