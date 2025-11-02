#pragma once

#include "Titan/PCH.h"

namespace Titan
{
    class TI_API Mesh
    {
    public:
        Mesh() = default;
        ~Mesh() = default;

        const std::vector<glm::vec3>& GetPositions() const { return m_Positions; }
        const std::vector<glm::vec3>& GetNormals() const { return m_Normals; }
        const std::vector<glm::vec2>& GetTexCoords() const { return m_TexCoords; }

        static Ref<Mesh> Create(const std::string& filepath);

    private:
        std::vector<glm::vec3> m_Positions;
        std::vector<glm::vec3> m_Normals;
        std::vector<glm::vec2> m_TexCoords;

        friend class Renderer3D;
    };
} // namespace Titan