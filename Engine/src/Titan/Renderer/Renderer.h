#pragma once

#include "Camera.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "VertexArray.h"

namespace Titan
{

    class TI_API Renderer
    {
    public:
        static void BeginScene(OrthographicCamera& camera);
        static void EndScene();

        static void Submit(const std::shared_ptr<VertexArray>& vertexArray, const std::shared_ptr<Shader>& shader);

        inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

    private:
        struct SceneData
        {
            glm::mat4 ViewProjMatrix;
        };

        static SceneData* m_Data;
    };

} // namespace Titan