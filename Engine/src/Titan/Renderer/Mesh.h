#pragma once

#include "Material.h"
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
        const std::vector<glm::vec3>& GetTangents() const { return m_Tangents; }
        const std::vector<glm::vec2>& GetTexCoords() const { return m_TexCoords; }
        const std::vector<uint8_t>& GetMaterialIndices() const { return m_MaterialIndex; }
        const std::vector<Ref<Material3D>>& GetMaterials() const { return m_Materials; }
        const Ref<Material3D>& GetMaterial(int index) const { return m_Materials[index]; }
        const std::string& GetFilePath() const { return m_FilePath; }

        static Ref<Mesh> CreateQuad();
        static Ref<Mesh> CreateCube();
        static Ref<Mesh> Create(const std::string& filepath);

    private:
        std::vector<glm::vec3> m_Positions;
        std::vector<glm::vec3> m_Normals;
        std::vector<glm::vec2> m_TexCoords;
        std::vector<glm::vec3> m_Tangents;
        std::vector<Ref<Material3D>> m_Materials;
        std::vector<uint8_t> m_MaterialIndex;

        std::string m_FilePath;

        friend class Renderer3D;
    };
} // namespace Titan