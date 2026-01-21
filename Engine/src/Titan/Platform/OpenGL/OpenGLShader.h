#pragma once
#include <slang-com-ptr.h>
#include <slang.h>
#include <glm/glm.hpp>
#include "Titan/Renderer/Shader.h"

typedef unsigned int GLenum;
typedef int GLint;

namespace Titan
{
    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader(const std::string& filepath);
        OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
        OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& geometrySrc,
                     const std::string& fragmentSrc);
        virtual ~OpenGLShader();

        virtual const std::string& GetName() const override { return m_Name; }
        virtual void Reload(const std::string& path) override;

        uint32_t GetRendererID() const { return m_RendererID; }

    private:
        std::unordered_map<GLenum, std::string> ParseShaderFile(const std::string& source);
        void Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
        void CompileSlangShader(const std::string& filepath);
        std::string CompileSlangEntryPoint(Slang::ComPtr<slang::ISession> session, slang::IModule* module,
                                           Slang::ComPtr<slang::IEntryPoint> entryPoint,
                                           const std::string& entryPointName);
        GLint GetUniformLocation(const std::string& name);

    private:
        uint32_t m_RendererID;
        std::string m_Name;
    };
} // namespace Titan