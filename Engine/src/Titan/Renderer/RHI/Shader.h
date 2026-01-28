#pragma once

#include "Titan/Core.h"
#include "Titan/PCH.h"

namespace Titan
{

    class TI_API Shader
    {
    public:
        virtual ~Shader() = default;

        virtual const std::string& GetName() const = 0;

        // Reload shader from the given file path. Implementations should update the shader object
        // in-place so existing `Ref<Shader>` instances remain valid.
        virtual void Reload(const std::string& path) = 0;

        static Ref<Shader> Create(const std::string& name, const std::string& vertexSrc,
                                  const std::string& fragmentSrc);
        static Ref<Shader> Create(const std::string& path);
    };

    class TI_API ShaderLibrary
    {
    public:
        void Add(const Ref<Shader>& shader);
        void Add(const std::string& name, const Ref<Shader>& shader);
        Ref<Shader> Load(const std::string& filepath);
        Ref<Shader> Load(const std::string& name, const std::string& filepath);

        Ref<Shader> Get(const std::string& name);

    private:
        std::unordered_map<std::string, Ref<Shader>> m_Shaders;
    };

} // namespace Titan