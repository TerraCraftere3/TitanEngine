#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/EditorCamera.h"
#include "Titan/Renderer/Framebuffer.h"
#include "Titan/Renderer/GeometryRenderer.h"
#include "Titan/Renderer/Renderer2D.h"
#include "Titan/Scene/Scene.h"

namespace Titan
{
    struct OverlaySettings
    {
        bool enableOverlay = true;
        bool enableBoundingBoxRender = false;
        bool enableWireframe = false;
    };

    class TI_API SceneRenderer
    {
    public:
        static constexpr uint32_t kMaxViews = 4;

        static void Init();
        static void Shutdown();

        // Single-view (backward compatible, maps to view 0)
        static void RenderSceneRuntime(Ref<Scene> scene);
        static void RenderSceneEditor(Ref<Scene> scene, EditorCamera& camera, OverlaySettings overlay = {});
        static void Resize(uint32_t width, uint32_t height);
        static Ref<Framebuffer> GetFramebuffer();

        // Multi-view variants
        static void RenderSceneRuntime(uint32_t viewIndex, Ref<Scene> scene);
        static void RenderSceneEditor(uint32_t viewIndex, Ref<Scene> scene, EditorCamera& camera,
                                      OverlaySettings overlay = {});
        static void Resize(uint32_t viewIndex, uint32_t width, uint32_t height);
        static Ref<Framebuffer> GetFramebuffer(uint32_t viewIndex);

    private:
        static void SetupRenderGraph(uint32_t viewIndex);
        static void EnsureView(uint32_t viewIndex);
    };
} // namespace Titan