#include "Renderer.h"
#include "Titan/PCH.h"

namespace Titan
{
    Renderer::SceneData* Renderer::m_Data = new Renderer::SceneData;

    void Renderer::BeginScene(OrthographicCamera& camera)
    {
        m_Data->ViewProjMatrix = camera.GetViewProjectionMatrix();
    }

    void Renderer::EndScene() {}

    void Renderer::Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader,
                          const glm::mat4& transform)
    {
        shader->Bind();
        shader->SetUniformMat4("u_ViewProjection", m_Data->ViewProjMatrix);
        shader->SetUniformMat4("u_Model", transform);
        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }

} // namespace Titan