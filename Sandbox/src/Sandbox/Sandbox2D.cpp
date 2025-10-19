#include "Sandbox2D.h"

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

Sandbox2D::Sandbox2D() : Layer("Sandbox2D Test"), m_CameraController(1280.0f / 720.0f) {}

void Sandbox2D::OnAttach()
{
    m_SquareVA = Titan::VertexArray::Create();

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

    m_Shader = m_Shaders.Load("shaders/texture.glsl");

    m_Texture = Titan::Texture2D::Create("textures/checkerboard.png");
    m_LogoTexture = Titan::Texture2D::Create("textures/google_logo.png");

    std::dynamic_pointer_cast<Titan::OpenGLShader>(m_Shader)->Bind();
    std::dynamic_pointer_cast<Titan::OpenGLShader>(m_Shader)->UploadUniformInt("u_Texture", 0);
}

void Sandbox2D::OnDetach() {}

void Sandbox2D::OnUpdate(Titan::Timestep ts)
{
    Titan::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    Titan::RenderCommand::Clear();

    m_CameraController.OnUpdate(ts);

    Titan::Renderer::BeginScene(m_CameraController.GetCamera());

    m_Texture->Bind();
    Titan::Renderer::Submit(m_SquareVA, m_Shader, transformationMatrix);
    m_LogoTexture->Bind();
    Titan::Renderer::Submit(m_SquareVA, m_Shader, transformationMatrix);

    Titan::Renderer::EndScene();
}

void Sandbox2D::OnEvent(Titan::Event& event)
{
    m_CameraController.OnEvent(event);
}

void Sandbox2D::OnImGuiRender(ImGuiContext* ctx)
{
    ImGui::SetCurrentContext(ctx);
    ImGui::Begin("Test");
    EditTransformImGui(transformationMatrix);
    ImGui::End();
}