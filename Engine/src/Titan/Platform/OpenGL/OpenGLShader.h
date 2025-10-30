#pragma once

#include <slang-com-ptr.h>
#include <slang.h>
#include <glm/glm.hpp>
#include "Titan/Renderer/Shader.h"

namespace slang
{
    struct ISession;
    struct IModule;
} // namespace slang

namespace Slang
{
    template <typename T>
    class ComPtr;
}

namespace Titan
{

    class TI_API OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& filepath);
        OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
        virtual ~OpenGLShader();

        void Bind() const override;
        void Unbind() const override;

        void SetBool(const std::string& name, bool value);
        void SetInt(const std::string& name, int value);
        void SetIntArray(const std::string& name, int* values, uint32_t count);

        void SetFloat(const std::string& name, float value);
        void SetFloat2(const std::string& name, const glm::vec2& value);
        void SetFloat3(const std::string& name, const glm::vec3& value);
        void SetFloat4(const std::string& name, const glm::vec4& value);

        void SetMat2(const std::string& name, const glm::mat2& value);
        void SetMat3(const std::string& name, const glm::mat3& value);
        void SetMat4(const std::string& name, const glm::mat4& value);

        const std::string& GetName() const { return m_Name; };

    private:
        void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
        void CompileSlangShader(const std::string& filepath);
        std::string CompileSlangEntryPoint(Slang::ComPtr<slang::ISession> session, slang::IModule* module,
                                           Slang::ComPtr<slang::IEntryPoint> entryPoint,
                                           const std::string& entryPointName);
        std::unordered_map<GLenum, std::string> ParseShaderFile(const std::string& source);

    private:
        std::string m_Name;
        uint32_t m_RendererID;
    };

} // namespace Titan