#pragma once

#include "Titan/PCH.h"

namespace Titan
{

    class Shader
    {
    public:
        Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
        ~Shader();

        void Bind() const;
        void Unbind() const;

        void SetUniformMat4(std::string location, const glm::mat4& matrix);

    private:
        uint32_t m_ID;
    };

} // namespace Titan