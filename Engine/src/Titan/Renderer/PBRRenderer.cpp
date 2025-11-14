#include "PBRRenderer.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Assets.h"
#include "UniformBuffer.h"
#include "VertexArray.h"
#include "Buffer.h"

namespace Titan {

    struct PBRRendererData {
        Ref<Shader> Shader;
        Ref<UniformBuffer> SceneUniformBuffer;
        Ref<VertexArray> FullscreenQuadVAO;
    };

    static PBRRendererData s_PBRData;

    void PBRRenderer::Init() {
        TI_PROFILE_FUNCTION();

        s_PBRData.SceneUniformBuffer = UniformBuffer::Create(sizeof(PBRSceneData), 0);

        // Load shader
        s_PBRData.Shader = Shader::Create("assets/shader/RendererPBR.slang");

        // Create fullscreen quad geometry
        float quadVertices[] = {
            // positions   // texcoords
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f,  1.0f, 1.0f,
            -1.0f,  1.0f,  0.0f, 1.0f
        };

        uint32_t quadIndices[] = {
            0, 1, 2,
            2, 3, 0
        };

        s_PBRData.FullscreenQuadVAO = VertexArray::Create();

        Ref<VertexBuffer> vb = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
        vb->SetLayout({
            { ShaderDataType::Float2, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });
        s_PBRData.FullscreenQuadVAO->AddVertexBuffer(vb);

        Ref<IndexBuffer> ib = IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));
        s_PBRData.FullscreenQuadVAO->SetIndexBuffer(ib);
    }

    void PBRRenderer::Shutdown() {
        TI_PROFILE_FUNCTION();
        s_PBRData = {};
    }

    void PBRRenderer::Render(Ref<Framebuffer> gbuffer, PBRSceneData data) {
        TI_PROFILE_FUNCTION();

        s_PBRData.SceneUniformBuffer->SetData(&data, sizeof(PBRSceneData));

        s_PBRData.Shader->Bind();
        gbuffer->BindTexture(0, 1); // Position
        gbuffer->BindTexture(1, 2); // Normal
        gbuffer->BindTexture(2, 3); // Albedo
        gbuffer->BindTexture(3, 4); // Metallic, Roughness, /, /
        gbuffer->BindTexture(4, 5); // Entity ID
        gbuffer->BindDepthTexture(6),

        s_PBRData.SceneUniformBuffer->Bind();

        RenderCommand::DrawIndexed(s_PBRData.FullscreenQuadVAO);
    }

} // namespace Titan
