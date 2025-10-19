#include "Renderer.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"

namespace Titan
{
    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

    void Renderer::Init()
    {
        RenderCommand::Init();
    }

    void Renderer::BeginScene(OrthographicCamera& camera)
    {
        s_SceneData->ViewProjMatrix = camera.GetViewProjectionMatrix();
    }

    void Renderer::EndScene() {}

    void Renderer::Submit(const Ref<VertexArray>& vertexArray, const Ref<Shader>& shader, const glm::mat4& transform)
    {
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