#pragma once

#include "Buffer.h"
#include "Titan/Core.h"
#include "Titan/PCH.h"

namespace Titan
{

    class TI_API VertexArray
    {
    public:
        virtual ~VertexArray() {}

        virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
        virtual void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

        virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
        virtual const Ref<IndexBuffer>& GetIndexBuffer() const = 0;

        static Ref<VertexArray> Create();
    };
} // namespace Titan