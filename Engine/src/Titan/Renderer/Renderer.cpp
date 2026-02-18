#include "Renderer.h"
#include "RHI/PipelineState.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"
#include "Titan/Renderer/Blit.h"
#include "Titan/Renderer/Systems/FullscreenRenderer.h"
#include "Titan/Renderer/Systems/GeometryRenderer.h"
#include "Titan/Renderer/Systems/PBRRenderer.h"
#include "Titan/Renderer/Systems/Renderer2D.h"
#include "Titan/Renderer/Systems/SkyboxRenderer.h"
#include "Titan/Renderer/Systems/TerrainRenderer.h"

namespace Titan
{

    void Renderer::Init()
    {
        RenderCommand::Init();
        Renderer2D::Init();
        GeometryRenderer::Init();
        TerrainRenderer::Init();
        PBRRenderer::Init();
        FullscreenRenderer::Init();
        SkyboxRenderer::Init();
        Blit::Init();
    }

    void Renderer::Shutdown()
    {
        Blit::Shutdown();
        SkyboxRenderer::Shutdown();
        PBRRenderer::Shutdown();
        TerrainRenderer::Shutdown();
        GeometryRenderer::Shutdown();
        Renderer2D::Shutdown();
        FullscreenRenderer::Shutdown();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

} // namespace Titan