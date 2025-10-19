#include "Renderer.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"

namespace Titan
{
    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

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

} // namespace Titan