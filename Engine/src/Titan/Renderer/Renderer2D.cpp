#include "Renderer2D.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"
#include "VertexArray.h"

namespace Titan
{

    struct Renderer2DStorage
    {
        Ref<VertexArray> QuadVertexArray;
        Ref<Shader> FlatColorShader;
    };

    static Renderer2DStorage* s_Data;

    void Renderer2D::Init()
    {
        s_Data = new Renderer2DStorage();
        s_Data->QuadVertexArray = VertexArray::Create();

        float squareVertices[3 * 4] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f};

        auto squareVB = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
        squareVB->SetLayout({{ShaderDataType::Float3, "a_Position"}});
        s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        auto squareIB = (IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
        s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

        s_Data->FlatColorShader = Shader::Create("shader/color.glsl");
    }

    void Renderer2D::Shutdown()
    {
        delete s_Data;
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)
            ->UploadUniformMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
    }

    void Renderer2D::EndScene() {}

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        glm::mat4 translation(1.0f);
        translation = glm::translate(translation, position);
        translation = glm::scale(translation, glm::vec3(size, 1.0f));
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformFloat4("u_Color", color);
        std::dynamic_pointer_cast<OpenGLShader>(s_Data->FlatColorShader)->UploadUniformMat4("u_Model", translation);

        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }

} // namespace Titan