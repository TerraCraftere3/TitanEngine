#include "OpenGLShader.h"
#include <filesystem>
#include "Titan/PCH.h"
// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace Titan
{
    std::string ReadFile(const std::string& filepath)
    {
        std::string result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        if (in)
        {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
            in.close();
        }
        else
        {
            TI_CORE_ERROR("Could not open file '{}'", filepath);
        }
        return result;
    }

    std::string GetPathWithoutExtension(const std::string& pathStr)
    {
        std::filesystem::path path(pathStr);
        return (path.parent_path() / path.stem()).string();
    }

    static GLenum ShaderTypeFromString(const std::string& type)
    {
        if (type == "vertex")
            return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel")
            return GL_FRAGMENT_SHADER;

        TI_CORE_ASSERT(false, "Unknown shader type '{}'", type);
        return 0;
    }

    OpenGLShader::OpenGLShader(const std::string& filepath)
    {
        TI_PROFILE_FUNCTION();
        m_Name = filepath;

        // Check if it's a Slang shader
        std::filesystem::path path(filepath);
        if (path.extension() == ".slang")
        {
            CompileSlangShaderToSPIRV(filepath);
        }
        else
        {
            // Legacy path for GLSL shaders
            std::string source = ReadFile(filepath);
            std::unordered_map<GLenum, std::string> shaderSources = ParseShaderFile(source);
            Compile(shaderSources);
        }
    }

    OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        TI_PROFILE_FUNCTION();
        m_Name = name;
        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER] = vertexSrc;
        sources[GL_FRAGMENT_SHADER] = fragmentSrc;
        Compile(sources);
    }

    OpenGLShader::~OpenGLShader()
    {
        TI_PROFILE_FUNCTION();
        glDeleteProgram(m_RendererID);
    }

    void OpenGLShader::Bind() const
    {
        glUseProgram(m_RendererID);
    }

    void OpenGLShader::Unbind() const
    {
        glUseProgram(0);
    }

    void OpenGLShader::SetBool(const std::string& name, bool value)
    {
        GLint location = GetUniformLocation(name);
        glUniform1i(location, (int)value);
    }

    void OpenGLShader::SetInt(const std::string& name, int value)
    {
        GLint location = GetUniformLocation(name);
        glUniform1i(location, value);
    }

    void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
    {
        GLint location = GetUniformLocation(name);
        glUniform1iv(location, count, values);
    }

    void OpenGLShader::SetFloat(const std::string& name, float value)
    {
        GLint location = GetUniformLocation(name);
        glUniform1f(location, value);
    }

    void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
    {
        GLint location = GetUniformLocation(name);
        glUniform2f(location, value.x, value.y);
    }

    void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
    {
        GLint location = GetUniformLocation(name);
        glUniform3f(location, value.x, value.y, value.z);
    }

    void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
    {
        GLint location = GetUniformLocation(name);
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }

    void OpenGLShader::SetMat2(const std::string& name, const glm::mat2& value)
    {
        GLint location = GetUniformLocation(name);
        glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& value)
    {
        GLint location = GetUniformLocation(name);
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
    {
        GLint location = GetUniformLocation(name);
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

    std::unordered_map<GLenum, std::string> OpenGLShader::ParseShaderFile(const std::string& source)
    {
        std::unordered_map<GLenum, std::string> shaderSources;

        const char* typeToken = "#shader";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = source.find(typeToken, 0);

        while (pos != std::string::npos)
        {
            size_t eol = source.find_first_of("\r\n", pos);
            TI_CORE_ASSERT(eol != std::string::npos, "Syntax error: missing end of line after {}", typeToken);

            size_t begin = pos + typeTokenLength + 1;
            std::string type = source.substr(begin, eol - begin);
            GLenum shaderType = ShaderTypeFromString(type);
            TI_CORE_ASSERT(shaderType, "Invalid shader type specified");

            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            TI_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error: missing shader code after {} line",
                           typeToken);

            pos = source.find(typeToken, nextLinePos);
            size_t endPos = (pos == std::string::npos) ? source.size() : pos;

            shaderSources[shaderType] = source.substr(nextLinePos, endPos - nextLinePos);
        }

        return shaderSources;
    }

    void OpenGLShader::CompileSlangShaderToSPIRV(const std::string& filepath)
    {
        TI_PROFILE_FUNCTION();

        // Create Slang global session
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        if (SLANG_FAILED(slang::createGlobalSession(globalSession.writeRef())))
        {
            TI_CORE_ERROR("Failed to create Slang global session");
            return;
        }

        // Create session description for SPIR-V target
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = globalSession->findProfile("spirv_1_5");
        targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM;

        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        // Create session
        Slang::ComPtr<slang::ISession> session;
        if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
        {
            TI_CORE_ERROR("Failed to create Slang session");
            return;
        }

        // Load the Slang module
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        slang::IModule* module = session->loadModule(filepath.c_str(), diagnosticsBlob.writeRef());

        if (diagnosticsBlob)
        {
            TI_CORE_WARN("Slang diagnostics: {}", (const char*)diagnosticsBlob->getBufferPointer());
        }

        if (!module)
        {
            TI_CORE_ERROR("Failed to load Slang module: {}", filepath);
            return;
        }

        // Find entry points and compile to SPIR-V
        std::unordered_map<GLenum, std::vector<uint32_t>> compiledShaders;

        // Compile vertex shader
        Slang::ComPtr<slang::IEntryPoint> vertexEntry;
        if (SLANG_SUCCEEDED(module->findEntryPointByName("vertexMain", vertexEntry.writeRef())))
        {
            auto spirvCode = CompileSlangEntryPointToSPIRV(session, module, vertexEntry, "vertexMain");
            if (!spirvCode.empty())
            {
                compiledShaders[GL_VERTEX_SHADER] = spirvCode;
            }
        }

        // Compile fragment shader
        Slang::ComPtr<slang::IEntryPoint> fragmentEntry;
        if (SLANG_SUCCEEDED(module->findEntryPointByName("fragmentMain", fragmentEntry.writeRef())))
        {
            auto spirvCode = CompileSlangEntryPointToSPIRV(session, module, fragmentEntry, "fragmentMain");
            if (!spirvCode.empty())
            {
                compiledShaders[GL_FRAGMENT_SHADER] = spirvCode;
            }
        }

        if (compiledShaders.empty())
        {
            TI_CORE_ERROR("No valid entry points found in Slang shader: {}", filepath);
            return;
        }

        // Load SPIR-V shaders into OpenGL
        CompileSPIRV(compiledShaders);
    }

    std::vector<uint32_t> OpenGLShader::CompileSlangEntryPointToSPIRV(Slang::ComPtr<slang::ISession> session,
                                                                      slang::IModule* module,
                                                                      Slang::ComPtr<slang::IEntryPoint> entryPoint,
                                                                      const std::string& entryPointName)
    {
        slang::IComponentType* componentTypes[] = {module, entryPoint};
        Slang::ComPtr<slang::IComponentType> program;
        if (SLANG_FAILED(session->createCompositeComponentType(componentTypes, 2, program.writeRef())))
        {
            TI_CORE_ERROR("Failed to create composite component type");
            return {};
        }

        // Link the program
        Slang::ComPtr<slang::IComponentType> linkedProgram;
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef())))
        {
            if (diagnosticsBlob)
            {
                TI_CORE_ERROR("Slang linking error: {}", (const char*)diagnosticsBlob->getBufferPointer());
            }
            return {};
        }

        // Get the compiled SPIR-V code
        Slang::ComPtr<slang::IBlob> codeBlob;
        if (SLANG_FAILED(linkedProgram->getEntryPointCode(0, 0, codeBlob.writeRef(), diagnosticsBlob.writeRef())))
        {
            if (diagnosticsBlob)
            {
                TI_CORE_ERROR("Slang code generation error: {}", (const char*)diagnosticsBlob->getBufferPointer());
            }
            return {};
        }

        // Convert blob to uint32_t vector (SPIR-V format)
        const uint32_t* spirvData = static_cast<const uint32_t*>(codeBlob->getBufferPointer());
        size_t spirvSize = codeBlob->getBufferSize() / sizeof(uint32_t);
        std::vector<uint32_t> spirvCode(spirvData, spirvData + spirvSize);

#ifdef TI_BUILD_DEBUG
        // Optionally save SPIR-V binary for debugging
        std::string outputPath = GetPathWithoutExtension(m_Name) + "_" + entryPointName + ".spv";
        std::ofstream out(outputPath, std::ios::binary);
        if (out)
        {
            out.write(reinterpret_cast<const char*>(spirvCode.data()), spirvCode.size() * sizeof(uint32_t));
            out.close();
            TI_CORE_TRACE("Wrote SPIR-V binary to: {}", outputPath);
        }
#endif

        return spirvCode;
    }

    GLint OpenGLShader::GetUniformLocation(const std::string& name)
    {
        return glGetUniformLocation(m_RendererID, (name + "_0").c_str());
    }

    void OpenGLShader::CompileSPIRV(const std::unordered_map<GLenum, std::vector<uint32_t>>& spirvShaders)
    {
        TI_PROFILE_FUNCTION();

        GLuint program = glCreateProgram();
        std::vector<GLuint> glShaderIDs;
        glShaderIDs.reserve(spirvShaders.size());

        for (const auto& [type, spirvCode] : spirvShaders)
        {
            GLuint shader = glCreateShader(type);

            // Load SPIR-V binary using GL_ARB_gl_spirv extension
            glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, spirvCode.data(),
                           static_cast<GLsizei>(spirvCode.size() * sizeof(uint32_t)));

            // Specialize the shader (required for SPIR-V)
            glSpecializeShader(shader, "main", 0, nullptr, nullptr);

            // Check for specialization errors
            GLint isSpecialized = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isSpecialized);
            if (isSpecialized == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(shader);

                TI_CORE_ERROR("SPIR-V shader specialization failure: {}", infoLog.data());
                TI_CORE_ASSERT(false, "SPIR-V shader specialization failure!");
                break;
            }

            glAttachShader(program, shader);
            glShaderIDs.push_back(shader);
        }

        m_RendererID = program;

        // Link the program
        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);

            for (auto id : glShaderIDs)
                glDeleteShader(id);

            TI_CORE_ERROR("SPIR-V program link failure: {}", infoLog.data());
            TI_CORE_ASSERT(false, "SPIR-V program link failure!");
            return;
        }

        // Detach shaders after linking
        for (auto id : glShaderIDs)
        {
            glDetachShader(program, id);
            glDeleteShader(id);
        }
    }

    void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
    {
        GLuint program = glCreateProgram();
        std::vector<GLenum> glShaderIDs;
        glShaderIDs.reserve(shaderSources.size());

        for (auto& kv : shaderSources)
        {
            GLenum type = kv.first;
            const std::string& source = kv.second;

            GLuint shader = glCreateShader(type);

            const GLchar* sourceCStr = source.c_str();
            glShaderSource(shader, 1, &sourceCStr, 0);

            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(shader);

                TI_CORE_ERROR("{0}", infoLog.data());
                TI_CORE_ASSERT(false, "Shader compilation failure!");
                break;
            }

            glAttachShader(program, shader);
            glShaderIDs.push_back(shader);
        }

        m_RendererID = program;

        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);

            for (auto id : glShaderIDs)
                glDeleteShader(id);

            TI_CORE_ERROR("{0}", infoLog.data());
            TI_CORE_ASSERT(false, "Shader link failure!");
            return;
        }

        for (auto id : glShaderIDs)
            glDetachShader(program, id);
    }

} // namespace Titan