#pragma once

#include "Buffer.h"
#include "Cubemap.h"
#include "Shader.h"
#include "Texture.h"
#include "Titan/Core.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

namespace Titan
{
    /**
     * @class PipelineState
     * @brief Encapsulates the entire rendering pipeline state for a single draw call.
     *
     * PipelineState combines all the rendering state that needs to be bound together:
     * - Shader program
     * - Vertex Array (VAO)
     * - Uniform Buffers
     * - Shader Storage Buffers
     * - Textures and their bindings
     *
     * This design allows for a single Bind() call to set up all necessary state,
     * making the code cleaner and more efficient. This also prepares the architecture
     * for future multi-API support (Vulkan, DirectX) where pipeline state is more
     * explicit and important.
     */
    class TI_API PipelineState
    {
    public:
        virtual ~PipelineState() = default;

        /**
         * @brief Binds all pipeline state (shader, VAO, uniform buffers, SSBOs, textures)
         */
        virtual void Bind() const = 0;

        // Setters for pipeline components
        virtual void SetShader(const Ref<Shader>& shader) = 0;
        virtual void SetVertexArray(const Ref<VertexArray>& vertexArray) = 0;

        /**
         * @brief Bind a uniform buffer at a specific binding point
         * @param uniformBuffer The uniform buffer to bind
         * @param bindingPoint The binding point (e.g., 0 for camera, 1 for materials, etc.)
         */
        virtual void BindUniformBuffer(const Ref<UniformBuffer>& uniformBuffer, uint32_t bindingPoint) = 0;

        /**
         * @brief Bind a shader storage buffer at a specific binding point
         * @param storageBuffer The shader storage buffer to bind
         * @param bindingPoint The binding point
         */
        virtual void BindShaderStorageBuffer(const Ref<ShaderStorageBuffer>& storageBuffer, uint32_t bindingPoint) = 0;

        /**
         * @brief Bind a texture to a texture unit
         * @param texture The texture to bind
         * @param slot The texture unit/slot (0-31 typically)
         */
        virtual void BindTexture(const Ref<Texture2D>& texture, uint32_t slot) = 0;

        /**
         * @brief Bind a cubemap to a texture unit
         * @param cubemap The cubemap to bind
         * @param slot The texture unit/slot
         */
        virtual void BindCubemap(const Ref<Cubemap>& cubemap, uint32_t slot) = 0;

        /**
         * @brief Clear all bound resources from the previous frame
         * Must be called before binding new resources each frame
         */
        virtual void ClearBindings() = 0;

        // Getters
        virtual Ref<Shader> GetShader() const = 0;
        virtual Ref<VertexArray> GetVertexArray() const = 0;

        static Ref<PipelineState> Create();
    };
} // namespace Titan
