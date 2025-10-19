#include <Titan.h>
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

    // --- Fix: Prevent scale from becoming 0 or negative ---
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
        m_VertexArray.reset(Titan::VertexArray::Create());

        float vertices[3 * 7] = {-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f, 0.5f, -0.5f, 0.0f, 0.2f,
                                 0.3f,  0.8f,  1.0f, 0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f,  1.0f};

        Titan::Ref<Titan::VertexBuffer> vertexBuffer;
        vertexBuffer.reset(Titan::VertexBuffer::Create(vertices, sizeof(vertices)));
        Titan::BufferLayout layout = {{Titan::ShaderDataType::Float3, "a_Position"},
                                      {Titan::ShaderDataType::Float4, "a_Color"}};
        vertexBuffer->SetLayout(layout);
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[3] = {0, 1, 2};
        Titan::Ref<Titan::IndexBuffer> indexBuffer;
        indexBuffer.reset(Titan::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
        m_VertexArray->SetIndexBuffer(indexBuffer);

        m_SquareVA.reset(Titan::VertexArray::Create());

        float squareVertices[3 * 4] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f};

        Titan::Ref<Titan::VertexBuffer> squareVB;
        squareVB.reset(Titan::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
        squareVB->SetLayout({{Titan::ShaderDataType::Float3, "a_Position"}});
        m_SquareVA->AddVertexBuffer(squareVB);

        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        Titan::Ref<Titan::IndexBuffer> squareIB;
        squareIB.reset(Titan::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
        m_SquareVA->SetIndexBuffer(squareIB);

        std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_Model;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);	
			}
		)";

        std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

        m_Shader.reset(Titan::Shader::Create(vertexSrc, fragmentSrc));

        std::string blueShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

            uniform mat4 u_ViewProjection;
            uniform mat4 u_Model;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);	
			}
		)";

        std::string blueShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		)";

        m_BlueShader.reset(Titan::Shader::Create(blueShaderVertexSrc, blueShaderFragmentSrc));
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

        glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
        float spacing = 0.11f;
        int gridSize = 20;
        glm::vec2 gridOffset = glm::vec2(gridSize, gridSize) * spacing * 0.5f;

        for (int y = 0; y < gridSize; y++)
        {
            for (int x = 0; x < gridSize; x++)
            {
                glm::vec3 pos(x * spacing - gridOffset.x, y * spacing - gridOffset.y, 0.0f);
                glm::mat4 transform = transformationMatrix * glm::translate(glm::mat4(1.0f), pos) * scale;
                Titan::Renderer::Submit(m_SquareVA, m_BlueShader, transform);
            }
        }

        Titan::Renderer::Submit(m_VertexArray, m_Shader, transformationMatrix);

        Titan::Renderer::EndScene();
    }

    virtual void OnEvent(Titan::Event& event) override {}

    virtual void OnImGuiRender(ImGuiContext* ctx) override
    {
        ImGui::SetCurrentContext(ctx);
        ImGui::Begin("Controller");
        static float fltest = 0.0f;
        ImGui::DragFloat("Movement Speed", &m_MovementSpeed, 0.01f, 0.5f, 5.0f);
        EditTransformImGui(transformationMatrix);
        ImGui::End();
    }

private:
    Titan::Ref<Titan::Shader> m_Shader;
    Titan::Ref<Titan::VertexArray> m_VertexArray;

    Titan::Ref<Titan::Shader> m_BlueShader;
    Titan::Ref<Titan::VertexArray> m_SquareVA;

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