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
        Ref<Shader> TextureShader;
    };

    static Renderer2DStorage* s_Data;

    void Renderer2D::Init()
    {
        s_Data = new Renderer2DStorage();
        s_Data->QuadVertexArray = VertexArray::Create();

        float squareVertices[5 * 4] = {-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.0f,
                                       0.5f,  0.5f,  0.0f, 1.0f, 1.0f, -0.5f, 0.5f,  0.0f, 0.0f, 1.0f};

        auto squareVB = VertexBuffer::Create(squareVertices, sizeof(squareVertices));
        squareVB->SetLayout({{ShaderDataType::Float3, "a_Position"}, {ShaderDataType::Float2, "a_TexCoord"}});

        s_Data->QuadVertexArray->AddVertexBuffer(squareVB);

        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        auto squareIB = (IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
        s_Data->QuadVertexArray->SetIndexBuffer(squareIB);

        s_Data->FlatColorShader = Shader::Create("shader/color.glsl");
        s_Data->TextureShader = Shader::Create("shader/texture.glsl");
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetInt("u_Texture", 0);
    }

    void Renderer2D::Shutdown()
    {
        delete s_Data;
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        s_Data->FlatColorShader->Bind();
        s_Data->FlatColorShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
    }

    void Renderer2D::EndScene() {}

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        glm::mat4 transform(1.0f);
        transform = glm::translate(transform, position);
        transform = glm::scale(transform, glm::vec3(size, 1.0f));
        s_Data->FlatColorShader->Bind();
        s_Data->FlatColorShader->SetFloat4("u_Color", color);
        s_Data->FlatColorShader->SetMat4("u_Model", transform);

        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture)
    {
        DrawQuad({position.x, position.y, 0.0f}, size, texture);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture)
    {
        glm::mat4 transform(1.0f);
        transform = glm::translate(transform, position);
        transform = glm::scale(transform, glm::vec3(size, 1.0f));
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_Model", transform);

        texture->Bind();

        s_Data->QuadVertexArray->Bind();
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray);
    }

} // namespace Titan