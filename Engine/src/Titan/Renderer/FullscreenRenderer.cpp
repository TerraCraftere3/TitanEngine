#include "FullscreenRenderer.h"
#include "Buffer.h"
#include "Titan/PCH.h"

namespace Titan
{

    FullscreenRenderer::Data FullscreenRenderer::s_Data;

    void FullscreenRenderer::Init()
    {
        // Fullscreen quad vertices
        float quadVertices[] = {
            // positions   // texcoords
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };

        uint32_t quadIndices[] = { 0, 1, 2, 2, 3, 0 };

        s_Data.FullscreenQuadVAO = VertexArray::Create();

        // Vertex Buffer
        Ref<VertexBuffer> vb = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
        vb->SetLayout({
            { ShaderDataType::Float2, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" }
        });
        s_Data.FullscreenQuadVAO->AddVertexBuffer(vb);

        // Index Buffer
        Ref<IndexBuffer> ib = IndexBuffer::Create(quadIndices, 6);
        s_Data.FullscreenQuadVAO->SetIndexBuffer(ib);
    }

    void FullscreenRenderer::Shutdown()
    {
        s_Data = {};
    }

    void FullscreenRenderer::Render()
    {
        RenderCommand::DrawIndexed(s_Data.FullscreenQuadVAO);
    }

}
