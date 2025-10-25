#include "Renderer.h"
#include "Renderer2D.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"

namespace Titan
{
    Scope<Renderer::SceneData> Renderer::s_SceneData = CreateScope<Renderer::SceneData>();

    void Renderer::Init()
    {
        TI_PROFILE_FUNCTION();
        RenderCommand::Init();
        Renderer2D::Init();
    }

    void Renderer::BeginScene(Camera& camera, const glm::mat4& transform)
    {
        TI_PROFILE_FUNCTION();
        s_SceneData->ViewProjMatrix = camera.GetProjection() * glm::inverse(transform);
    }

    void Renderer::EndScene()
    {
        TI_PROFILE_FUNCTION();
    }

    void Renderer::Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shader, const glm::mat4& transform)
    {
        TI_PROFILE_FUNCTION();
        shader->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection",
                                                                           s_SceneData->ViewProjMatrix);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Model", transform);
        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

} // namespace Titan