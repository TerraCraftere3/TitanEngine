#pragma once

#include <cstdint>
#include <vector>
#include <utility>
#include "Titan/Renderer/PipelineState.h"
#include "Titan/Renderer/Buffer.h"

namespace Titan
{
    class OpenGLPipelineState : public PipelineState
    {
    public:
        OpenGLPipelineState();
        virtual ~OpenGLPipelineState() = default;

        virtual void Bind() const override;

        virtual void SetShader(const Ref<Shader>& shader) override;
        virtual void SetVertexArray(const Ref<VertexArray>& vertexArray) override;
        virtual void BindUniformBuffer(const Ref<UniformBuffer>& uniformBuffer, uint32_t bindingPoint) override;
        virtual void BindShaderStorageBuffer(const Ref<ShaderStorageBuffer>& storageBuffer, uint32_t bindingPoint) override;
        virtual void BindTexture(const Ref<Texture2D>& texture, uint32_t slot) override;
        virtual void BindCubemap(const Ref<Cubemap>& cubemap, uint32_t slot) override;

        virtual Ref<Shader> GetShader() const override { return m_Shader; }
        virtual Ref<VertexArray> GetVertexArray() const override { return m_VertexArray; }

        void ClearBindings();

    private:
        Ref<Shader> m_Shader = nullptr;
        Ref<VertexArray> m_VertexArray = nullptr;
        std::vector<std::pair<Ref<UniformBuffer>, uint32_t>> m_UniformBuffers;
        std::vector<std::pair<Ref<ShaderStorageBuffer>, uint32_t>> m_StorageBuffers;
        std::vector<std::pair<Ref<Texture2D>, uint32_t>> m_Textures;
        std::vector<std::pair<Ref<Cubemap>, uint32_t>> m_Cubemaps;
    };
} // namespace Titan
