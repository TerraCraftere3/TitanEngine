#include "SkyboxRenderer.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"
#include "Titan/Renderer/RHI/Buffer.h"
#include "Titan/Renderer/RHI/PipelineState.h"
#include "Titan/Renderer/RHI/Shader.h"
#include "Titan/Renderer/RHI/UniformBuffer.h"
#include "Titan/Renderer/RHI/VertexArray.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Scene/Assets.h"
#include "Titan/Utils/PlatformUtils.h"

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
        float Time;
        float _pad2[3];
    };

    struct SkyboxRendererData
    {
        Ref<Shader> CubemapShader;
        Ref<Shader> ColorShader;
        Ref<Shader> NormalShader;
        Ref<PipelineState> CubemapPipeline;
        Ref<PipelineState> ColorPipeline;
        Ref<PipelineState> NormalPipeline;
        Ref<UniformBuffer> SceneUniformBuffer;
        Ref<VertexArray> CubeVAO;
    };

    static SkyboxRendererData s_SBData;

#define CUBE_MAX 20.0f

    // Cube vertices
    static float s_CubeVertices[] = {
        -CUBE_MAX, CUBE_MAX,  -CUBE_MAX, -CUBE_MAX, -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  -CUBE_MAX, -CUBE_MAX,
        CUBE_MAX,  -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  CUBE_MAX,  -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  -CUBE_MAX,

        -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  -CUBE_MAX, -CUBE_MAX, -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  -CUBE_MAX,
        -CUBE_MAX, CUBE_MAX,  -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  CUBE_MAX,  -CUBE_MAX, -CUBE_MAX, CUBE_MAX,

        CUBE_MAX,  -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  -CUBE_MAX, CUBE_MAX,  CUBE_MAX,  CUBE_MAX,  CUBE_MAX,
        CUBE_MAX,  CUBE_MAX,  CUBE_MAX,  CUBE_MAX,  CUBE_MAX,  -CUBE_MAX, CUBE_MAX,  -CUBE_MAX, -CUBE_MAX,

        -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  -CUBE_MAX, CUBE_MAX,  CUBE_MAX,  CUBE_MAX,  CUBE_MAX,  CUBE_MAX,
        CUBE_MAX,  CUBE_MAX,  CUBE_MAX,  CUBE_MAX,  -CUBE_MAX, CUBE_MAX,  -CUBE_MAX, -CUBE_MAX, CUBE_MAX,

        -CUBE_MAX, CUBE_MAX,  -CUBE_MAX, CUBE_MAX,  CUBE_MAX,  -CUBE_MAX, CUBE_MAX,  CUBE_MAX,  CUBE_MAX,
        CUBE_MAX,  CUBE_MAX,  CUBE_MAX,  -CUBE_MAX, CUBE_MAX,  CUBE_MAX,  -CUBE_MAX, CUBE_MAX,  -CUBE_MAX,

        -CUBE_MAX, -CUBE_MAX, -CUBE_MAX, -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  CUBE_MAX,  -CUBE_MAX, -CUBE_MAX,
        CUBE_MAX,  -CUBE_MAX, -CUBE_MAX, -CUBE_MAX, -CUBE_MAX, CUBE_MAX,  CUBE_MAX,  -CUBE_MAX, CUBE_MAX};

    void SkyboxRenderer::Init()
    {
        auto shaderDir = Filesystem::GetExecutableDirectory() / "resources" / "shader";
        s_SBData.CubemapShader = Assets::Load<Shader>((shaderDir / "RendererSkyboxHDRI.slang").string());
        s_SBData.ColorShader = Assets::Load<Shader>((shaderDir / "RendererSkyboxColor.slang").string());
        s_SBData.NormalShader = Assets::Load<Shader>((shaderDir / "RendererSkybox.slang").string());

#ifdef TI_BUILD_DEBUG
        Assets::AttachHotreloader<Shader>((shaderDir / "RendererSkyboxHDRI.slang").string());
        Assets::AttachHotreloader<Shader>((shaderDir / "RendererSkyboxColor.slang").string());
        Assets::AttachHotreloader<Shader>((shaderDir / "RendererSkybox.slang").string());
#endif

        s_SBData.SceneUniformBuffer = UniformBuffer::Create(sizeof(SkyboxSceneData));

        s_SBData.CubeVAO = VertexArray::Create();

        Ref<VertexBuffer> vb = VertexBuffer::Create(s_CubeVertices, sizeof(s_CubeVertices));
        vb->SetLayout({{ShaderDataType::Float3, "a_Position"}});
        s_SBData.CubeVAO->AddVertexBuffer(vb);

        // Create pipelines
        s_SBData.CubemapPipeline = PipelineState::Create();
        s_SBData.CubemapPipeline->SetShader(s_SBData.CubemapShader);
        s_SBData.CubemapPipeline->SetVertexArray(s_SBData.CubeVAO);
        s_SBData.CubemapPipeline->SetUniformBuffer(s_SBData.SceneUniformBuffer, 0);
        s_SBData.CubemapPipeline->SetDepthFunction(DepthFunc::LessEqual);

        s_SBData.ColorPipeline = PipelineState::Create();
        s_SBData.ColorPipeline->SetShader(s_SBData.ColorShader);
        s_SBData.ColorPipeline->SetVertexArray(s_SBData.CubeVAO);
        s_SBData.ColorPipeline->SetUniformBuffer(s_SBData.SceneUniformBuffer, 0);
        s_SBData.ColorPipeline->SetDepthFunction(DepthFunc::LessEqual);

        s_SBData.NormalPipeline = PipelineState::Create();
        s_SBData.NormalPipeline->SetShader(s_SBData.NormalShader);
        s_SBData.NormalPipeline->SetVertexArray(s_SBData.CubeVAO);
        s_SBData.NormalPipeline->SetUniformBuffer(s_SBData.SceneUniformBuffer, 0);
        s_SBData.NormalPipeline->SetDepthFunction(DepthFunc::LessEqual);
    }

    void SkyboxRenderer::Shutdown()
    {
        s_SBData.CubemapPipeline.reset();
        s_SBData.ColorPipeline.reset();
        s_SBData.NormalPipeline.reset();
        s_SBData = {};
    }

    void SkyboxRenderer::Render(Ref<Cubemap> cubemap, glm::mat4 view, glm::mat4 projection)
    {
        SkyboxSceneData data;
        data.View = view;
        data.Projection = projection;

        s_SBData.SceneUniformBuffer->SetData(&data, sizeof(data));
        s_SBData.CubemapPipeline->SetCubemap(cubemap, 0);
        s_SBData.CubemapPipeline->Bind();

        RenderCommand::DrawArrays(36);
        s_SBData.CubemapPipeline->ClearBindings();
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

        s_SBData.ColorPipeline->Bind();

        RenderCommand::DrawArrays(36);
        s_SBData.ColorPipeline->ClearBindings();
    }

    void SkyboxRenderer::Render(float time, glm::mat4 view, glm::mat4 projection)
    {
        SkyboxSceneData data;
        data.View = view;
        data.Projection = projection;
        data.Time = time;

        s_SBData.SceneUniformBuffer->SetData(&data, sizeof(data));

        s_SBData.NormalPipeline->Bind();

        RenderCommand::DrawArrays(36);
    }

} // namespace Titan
