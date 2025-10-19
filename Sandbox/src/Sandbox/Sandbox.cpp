#include <Titan.h>
#include <Titan/Platform/OpenGL/OpenGLShader.h>
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>

bool EditTransformImGui(glm::mat4& transform)
{
    using namespace glm;

    vec3 scale, translation, skew;
    vec4 perspective;
    quat orientation;

    if (!decompose(transform, scale, orientation, translation, skew, perspective))
        return false;

    vec3 rotation = degrees(eulerAngles(orientation));

    bool changed = false;
    if (ImGui::DragFloat3("Position", &translation.x, 0.1f))
        changed = true;
    if (ImGui::DragFloat3("Rotation", &rotation.x, 0.5f))
        changed = true;
    if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
        changed = true;

    const float minScale = 1e-4f;
    scale.x = max(scale.x, minScale);
    scale.y = max(scale.y, minScale);
    scale.z = max(scale.z, minScale);

    if (changed)
    {
        quat q = quat(radians(rotation));
        mat4 rotMat = toMat4(q);
        mat4 transMat = translate(mat4(1.0f), translation);
        mat4 scaleMat = glm::scale(mat4(1.0f), scale);
        transform = transMat * rotMat * scaleMat;
    }

    return changed;
}

class ExampleLayer : public Titan::Layer
{
public:
    ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
    {
        m_SquareVA.reset(Titan::VertexArray::Create());

        float squareVertices[5 * 4] = {
            // Position (x, y, z)    UV (u, v)
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // Bottom-left
            0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, // Bottom-right
            0.5f,  0.5f,  0.0f, 1.0f, 1.0f, // Top-right
            -0.5f, 0.5f,  0.0f, 0.0f, 1.0f  // Top-left
        };

        Titan::Ref<Titan::VertexBuffer> squareVB;
        squareVB.reset(Titan::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
        squareVB->SetLayout({{Titan::ShaderDataType::Float3, "a_Position"}, {Titan::ShaderDataType::Float2, "a_UV"}});
        m_SquareVA->AddVertexBuffer(squareVB);

        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        Titan::Ref<Titan::IndexBuffer> squareIB;
        squareIB.reset(Titan::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
        m_SquareVA->SetIndexBuffer(squareIB);

        std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec2 a_UV;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_Model;

			out vec2 v_UV;

			void main()
			{
				v_UV = a_UV;
				gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);	
			}
		)";

        std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec2 v_UV;

			uniform sampler2D u_Texture;

			void main()
			{
				color = texture(u_Texture, v_UV);
			}
		)";

        m_Shader.reset(Titan::Shader::Create(vertexSrc, fragmentSrc));

        m_Texture = Titan::Texture2D::Create("textures/Checkerboard.png");

        std::dynamic_pointer_cast<Titan::OpenGLShader>(m_Shader)->Bind();
        std::dynamic_pointer_cast<Titan::OpenGLShader>(m_Shader)->UploadUniformInt("u_Texture", 0);
    }

    virtual void OnUpdate(Titan::Timestep ts) override
    {
        Titan::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        Titan::RenderCommand::Clear();

        glm::vec3 movement(0.0f);
        if (Titan::Input::IsKeyPressed(TI_KEY_A))
            movement += glm::vec3(-1.0f, 0.0f, 0.0f);
        if (Titan::Input::IsKeyPressed(TI_KEY_D))
            movement += glm::vec3(1.0f, 0.0f, 0.0f);
        if (Titan::Input::IsKeyPressed(TI_KEY_W))
            movement += glm::vec3(0.0f, 1.0f, 0.0f);
        if (Titan::Input::IsKeyPressed(TI_KEY_S))
            movement += glm::vec3(0.0f, -1.0f, 0.0f);

        m_Camera.SetPosition(m_Camera.GetPosition() + movement * ts * m_MovementSpeed);

        Titan::Renderer::BeginScene(m_Camera);

        m_Texture->Bind();
        Titan::Renderer::Submit(m_SquareVA, m_Shader, transformationMatrix);

        Titan::Renderer::EndScene();
    }

    virtual void OnEvent(Titan::Event& event) override {}

    virtual void OnImGuiRender(ImGuiContext* ctx) override
    {
        ImGui::SetCurrentContext(ctx);
        ImGui::Begin("Controller");
        ImGui::DragFloat("Movement Speed", &m_MovementSpeed, 0.01f, 0.5f, 5.0f);
        EditTransformImGui(transformationMatrix);
        ImGui::End();
    }

private:
    Titan::Ref<Titan::Shader> m_Shader;
    Titan::Ref<Titan::VertexArray> m_SquareVA;
    Titan::Ref<Titan::Texture2D> m_Texture;

    Titan::OrthographicCamera m_Camera;
    glm::mat4 transformationMatrix = glm::mat4(1.0f);
    float m_MovementSpeed = 1.0f;
};

class Sandbox : public Titan::Application
{
public:
    Sandbox() { PushLayer(new ExampleLayer()); }
    ~Sandbox() {}
};

Titan::Application* Titan::CreateApplication()
{
    return new Sandbox();
}