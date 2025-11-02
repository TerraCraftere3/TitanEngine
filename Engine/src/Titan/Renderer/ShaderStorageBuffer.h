#pragma once
#include "Titan/Core.h"

namespace Titan
{
    class ShaderStorageBuffer
    {
    public:
        virtual ~ShaderStorageBuffer() = default;

        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;
        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        static Ref<ShaderStorageBuffer> Create(uint32_t size, uint32_t binding);
    };
}
