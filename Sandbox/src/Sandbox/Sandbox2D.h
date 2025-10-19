#pragma once
#include <Titan.h>
#include <Titan/Platform/OpenGL/OpenGLShader.h>
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
class Sandbox2D : public Titan::Layer
{
public:
    Sandbox2D();

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(Titan::Timestep ts) override;
    virtual void OnEvent(Titan::Event& event) override;
    virtual void OnImGuiRender(ImGuiContext* ctx) override;

private:
    Titan::ShaderLibrary m_Shaders;
    Titan::Ref<Titan::Shader> m_Shader;
    Titan::Ref<Titan::VertexArray> m_SquareVA;
    Titan::Ref<Titan::Texture2D> m_Texture, m_LogoTexture;

    Titan::OrthographicCameraController m_CameraController;
    glm::mat4 transformationMatrix = glm::mat4(1.0f);
};