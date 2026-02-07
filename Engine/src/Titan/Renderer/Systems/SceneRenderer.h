#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/EditorCamera.h"
#include "Titan/Renderer/RHI/Framebuffer.h"
#include "Titan/Renderer/Systems/GeometryRenderer.h"
#include "Titan/Renderer/Systems/Renderer2D.h"
#include "Titan/Scene/Scene.h"

namespace Titan
{
    struct SceneRendererData;

    struct OverlaySettings
    {
        bool enableOverlay = true;
        bool enableBoundingBoxRender = false;
        bool enableWireframe = false;
    };

    class TI_API SceneRenderer
    {
    public:
        static Ref<SceneRenderer> Create();

        SceneRenderer();
        ~SceneRenderer();

        void RenderSceneRuntime(Ref<Scene> scene);
        void RenderSceneEditor(Ref<Scene> scene, EditorCamera& camera, OverlaySettings overlay = {});
        void Resize(uint32_t width, uint32_t height);
        Ref<Framebuffer> GetFramebuffer();

    private:
        void SetupRenderGraph();
        void EnsureResources();
        Scope<SceneRendererData> m_Data;
    };
} // namespace Titan