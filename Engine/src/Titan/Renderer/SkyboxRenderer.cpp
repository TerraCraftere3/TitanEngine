#include "SkyboxRenderer.h"
#include "Buffer.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Assets.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

namespace Titan
{

    struct SkyboxSceneData
    {
        glm::mat4 View;
        glm::mat4 Projection;
        glm::vec3 TopColor;
        float _pad0;
        glm::vec3 BottomColor;
        float _pad1;
    };

    struct SkyboxRendererData
    {
        Ref<Shader> CubemapShader;
        Ref<Shader> ColorShader;
        Ref<UniformBuffer> SceneUniformBuffer;
        Ref<VertexArray> CubeVAO;
    };

    static SkyboxRendererData s_SBData;

    // Cube vertices
    static float s_CubeVertices[] = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                                     1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                                     -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                                     -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                                     1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                                     1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                                     -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                                     1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                                     -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                                     1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                                     -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                                     1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    void SkyboxRenderer::Init()
    {
        s_SBData.CubemapShader = Shader::Create("assets/shader/RendererSkyboxHDRI.slang");
        s_SBData.ColorShader = Shader::Create("assets/shader/RendererSkyboxColor.slang");

        s_SBData.SceneUniformBuffer = UniformBuffer::Create(sizeof(SkyboxSceneData), 0);

        s_SBData.CubeVAO = VertexArray::Create();

        Ref<VertexBuffer> vb = VertexBuffer::Create(s_CubeVertices, sizeof(s_CubeVertices));
        vb->SetLayout({{ShaderDataType::Float3, "a_Position"}});
        s_SBData.CubeVAO->AddVertexBuffer(vb);
    }

    void SkyboxRenderer::Shutdown()
    {
        s_SBData = {};
    }

    void SkyboxRenderer::Render(Ref<Cubemap> cubemap, glm::mat4 view, glm::mat4 projection)
    {
        SkyboxSceneData data;
        data.View = view;
        data.Projection = projection;

        s_SBData.SceneUniformBuffer->SetData(&data, sizeof(data));

        RenderCommand::SetDepthFunc(DepthFunc::LessEqual);

        s_SBData.CubemapShader->Bind();
        s_SBData.CubemapShader->SetInt("cubeMap", 0);
        s_SBData.SceneUniformBuffer->Bind();

        cubemap->Bind(0);

        RenderCommand::DrawArrays(s_SBData.CubeVAO, 36);

        RenderCommand::SetDepthFunc(DepthFunc::Less);
    }

    void SkyboxRenderer::Render(const glm::vec3& topColor, const glm::vec3& bottomColor, glm::mat4 view,
                                glm::mat4 projection)
    {
        SkyboxSceneData data;
        data.View = view;
        data.Projection = projection;
        data.TopColor = topColor;
        data.BottomColor = bottomColor;

        s_SBData.SceneUniformBuffer->SetData(&data, sizeof(data));

        RenderCommand::SetDepthFunc(DepthFunc::LessEqual);

        s_SBData.ColorShader->Bind();
        s_SBData.SceneUniformBuffer->Bind();

        RenderCommand::DrawArrays(s_SBData.CubeVAO, 36);

        RenderCommand::SetDepthFunc(DepthFunc::Less);
    }

} // namespace Titan
