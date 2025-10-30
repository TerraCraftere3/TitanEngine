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

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetBool(const std::string& name, bool value) override;
        virtual void SetInt(const std::string& name, int value) override;
        virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;

        virtual void SetFloat(const std::string& name, float value) override;
        virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
        virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
        virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;

        virtual void SetMat2(const std::string& name, const glm::mat2& value) override;
        virtual void SetMat3(const std::string& name, const glm::mat3& value) override;
        virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

        virtual const std::string& GetName() const override { return m_Name; };

    private:
        GLint GetUniformLocation(const std::string& name);
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