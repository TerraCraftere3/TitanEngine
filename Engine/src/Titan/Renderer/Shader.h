#pragma once

#include "Titan/Core.h"
#include "Titan/PCH.h"

namespace Titan
{

    class TI_API Shader
    {
    public:
        virtual ~Shader() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);
    };

} // namespace Titan