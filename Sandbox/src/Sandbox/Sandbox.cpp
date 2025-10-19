#include <Titan.h>

class ExampleLayer : public Titan::Layer
{
public:
    ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f)
    {
        m_VertexArray.reset(Titan::VertexArray::Create());

        float vertices[3 * 7] = {-0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f, 0.5f, -0.5f, 0.0f, 0.2f,
                                 0.3f,  0.8f,  1.0f, 0.0f, 0.5f, 0.0f, 0.8f, 0.8f, 0.2f,  1.0f};

        std::shared_ptr<Titan::VertexBuffer> vertexBuffer;
        vertexBuffer.reset(Titan::VertexBuffer::Create(vertices, sizeof(vertices)));
        Titan::BufferLayout layout = {{Titan::ShaderDataType::Float3, "a_Position"},
                                      {Titan::ShaderDataType::Float4, "a_Color"}};
        vertexBuffer->SetLayout(layout);
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        uint32_t indices[3] = {0, 1, 2};
        std::shared_ptr<Titan::IndexBuffer> indexBuffer;
        indexBuffer.reset(Titan::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));
        m_VertexArray->SetIndexBuffer(indexBuffer);

        m_SquareVA.reset(Titan::VertexArray::Create());

        float squareVertices[3 * 4] = {-0.75f, -0.75f, 0.0f, 0.75f,  -0.75f, 0.0f,
                                       0.75f,  0.75f,  0.0f, -0.75f, 0.75f,  0.0f};

        std::shared_ptr<Titan::VertexBuffer> squareVB;
        squareVB.reset(Titan::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
        squareVB->SetLayout({{Titan::ShaderDataType::Float3, "a_Position"}});
        m_SquareVA->AddVertexBuffer(squareVB);

        uint32_t squareIndices[6] = {0, 1, 2, 2, 3, 0};
        std::shared_ptr<Titan::IndexBuffer> squareIB;
        squareIB.reset(Titan::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
        m_SquareVA->SetIndexBuffer(squareIB);

        std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

            uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);	
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

        m_Shader.reset(new Titan::Shader(vertexSrc, fragmentSrc));

        std::string blueShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

            uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);	
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

        m_BlueShader.reset(new Titan::Shader(blueShaderVertexSrc, blueShaderFragmentSrc));
    }

    virtual void OnUpdate() override
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
        m_Camera.SetPosition(m_Camera.GetPosition() + movement * 0.01f);

        Titan::Renderer::BeginScene(m_Camera);
        Titan::Renderer::Submit(m_SquareVA, m_BlueShader);
        Titan::Renderer::Submit(m_VertexArray, m_Shader);
        Titan::Renderer::EndScene();
    }

    virtual void OnEvent(Titan::Event& event) override {}

    virtual void OnImGuiRender(ImGuiContext* ctx) override
    {
        ImGui::SetCurrentContext(ctx);
        ImGui::Begin("Test");
        static char text[1024] = "Hello, world!";
        ImGui::InputText("Text Input", text, IM_ARRAYSIZE(text));
        static float fltest = 0.0f;
        ImGui::DragFloat("Float Input", &fltest, 0.01f, -1.0f, 1.0f);
        static float coltest[4] = {1.0f, 0.0f, 1.0f, 1.0f};
        ImGui::ColorEdit4("Color Input", coltest);
        ImGui::End();
    }

private:
    std::shared_ptr<Titan::Shader> m_Shader;
    std::shared_ptr<Titan::VertexArray> m_VertexArray;

    std::shared_ptr<Titan::Shader> m_BlueShader;
    std::shared_ptr<Titan::VertexArray> m_SquareVA;

    Titan::OrthographicCamera m_Camera;
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