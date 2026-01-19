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

    void Renderer::BeginScene(Camera& camera, const glm::mat4& transform)
    {
        s_SceneData->ViewProjMatrix = camera.GetProjection() * glm::inverse(transform);
    }

    void Renderer::EndScene() {}

    void Renderer::Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shader, const glm::mat4& transform)
    {
        // Create a temporary pipeline state for this legacy submit call
        Ref<PipelineState> pipeline = PipelineState::Create();
        pipeline->SetShader(shader);
        pipeline->SetVertexArray(vertexArray);
        pipeline->Bind();

        // Set uniforms on shader directly
        std::dynamic_pointer_cast<OpenGLShader>(shader)->SetMat4("u_ViewProjection",
                                                                 s_SceneData->ViewProjMatrix); // LEGACY METHODS
        std::dynamic_pointer_cast<OpenGLShader>(shader)->SetMat4("u_Model", transform);        // TODO: REMOVE!!!

        RenderCommand::DrawIndexed();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

} // namespace Titan