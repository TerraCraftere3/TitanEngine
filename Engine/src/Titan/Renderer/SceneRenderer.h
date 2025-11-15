#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/EditorCamera.h"
#include "Titan/Renderer/Framebuffer.h"
#include "Titan/Renderer/Renderer2D.h"
#include "Titan/Renderer/Renderer3D.h"
#include "Titan/Scene/Scene.h"

namespace Titan
{

    class TI_API SceneRenderer
    {
    public:
        static void Init();
        static void Shutdown();
        static void RenderSceneRuntime(Ref<Scene> scene);
        static void RenderSceneEditor(Ref<Scene> scene, EditorCamera& camera);
        static void Resize(uint32_t width, uint32_t height);

        static Ref<Framebuffer> GetFramebuffer();

    private:
        static void SetupRenderGraph();
    };
} // namespace Titan