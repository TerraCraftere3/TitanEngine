#include "Renderer.h"
#include "FullscreenRenderer.h"
#include "GeometryRenderer.h"
#include "PBRRenderer.h"
#include "PipelineState.h"
#include "Renderer2D.h"
#include "SceneRenderer.h"
#include "SkyboxRenderer.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"

namespace Titan
{
    Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

    void Renderer::Init()
    {
        RenderCommand::Init();
        Renderer2D::Init();
        GeometryRenderer::Init();
        PBRRenderer::Init();
        FullscreenRenderer::Init();
        SkyboxRenderer::Init();
        SceneRenderer::Init();
    }

    void Renderer::Shutdown()
    {
        SceneRenderer::Shutdown();
        SkyboxRenderer::Shutdown();
        PBRRenderer::Shutdown();
        GeometryRenderer::Shutdown();
        Renderer2D::Shutdown();
        FullscreenRenderer::Shutdown();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

} // namespace Titan