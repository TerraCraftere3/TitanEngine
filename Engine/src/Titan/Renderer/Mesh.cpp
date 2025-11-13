#include "Mesh.h"
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

namespace Titan
{
    struct RawMeshData
    {
        std::vector<glm::vec3> Positions;
        std::vector<glm::vec3> Normals;
        std::vector<glm::vec2> TexCoords;
    };

    static void ProcessMesh(aiMesh* mesh, RawMeshData& data)
    {
        // Expand each triangle into unique vertices
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
            {
                unsigned int idx = face.mIndices[j];

                const aiVector3D& pos = mesh->mVertices[idx];
                data.Positions.emplace_back(pos.x, pos.y, pos.z);

                if (mesh->HasNormals())
                {
                    const aiVector3D& n = mesh->mNormals[idx];
                    data.Normals.emplace_back(n.x, n.y, n.z);
                }
                else
                    data.Normals.emplace_back(0.0f);

                if (mesh->HasTextureCoords(0))
                {
                    const aiVector3D& uv = mesh->mTextureCoords[0][idx];
                    data.TexCoords.emplace_back(uv.x, uv.y);
                }
                else
                    data.TexCoords.emplace_back(0.0f);
            }
        }
    }

    static void ProcessNode(aiNode* node, const aiScene* scene, RawMeshData& data)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(mesh, data);
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
            ProcessNode(node->mChildren[i], scene, data);
    }

    struct Vec3Hash
    {
        size_t operator()(const glm::vec3& v) const
        {
            size_t hx = std::hash<float>()(v.x);
            size_t hy = std::hash<float>()(v.y);
            size_t hz = std::hash<float>()(v.z);
            return hx ^ (hy << 1) ^ (hz << 2);
        }
    };

    void ComputeSmoothNormals(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals)
    {
        std::unordered_map<glm::vec3, glm::vec3, Vec3Hash> normalMap;

        for (size_t i = 0; i < positions.size(); i++)
            normalMap[positions[i]] += normals[i];

        for (size_t i = 0; i < positions.size(); i++)
            normals[i] = glm::normalize(normalMap[positions[i]]);
    }

    Ref<Mesh> Mesh::CreateQuad()
    {
        RawMeshData data;

        // Quad on XY plane, centered at origin
        data.Positions = {
            {-0.5f, -0.5f, 0.0f}, {0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f},
            {-0.5f, -0.5f, 0.0f}, {0.5f, 0.5f, 0.0f},  {-0.5f, 0.5f, 0.0f},
        };

        data.Normals = {
            {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f},
        };

        data.TexCoords = {
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f},
        };

        ComputeSmoothNormals(data.Positions, data.Normals);

        auto mesh = CreateRef<Mesh>();
        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_FilePath = "quad";
        return mesh;
    }

    Ref<Mesh> Mesh::CreateSubdivedQuad(uint32_t xSubdivisions, uint32_t ySubdivisions)
    {
        RawMeshData data;

        xSubdivisions = max(1u, xSubdivisions);
        ySubdivisions = max(1u, ySubdivisions);

        const float width = 1.0f;
        const float height = 1.0f;
        const float dx = width / xSubdivisions;
        const float dy = height / ySubdivisions;

        // Generate vertex data as triangle list
        for (uint32_t y = 0; y < ySubdivisions; ++y)
        {
            for (uint32_t x = 0; x < xSubdivisions; ++x)
            {
                float x0 = -0.5f + x * dx;
                float y0 = -0.5f + y * dy;
                float x1 = x0 + dx;
                float y1 = y0 + dy;

                glm::vec3 p00 = {x0, y0, 0.0f};
                glm::vec3 p10 = {x1, y0, 0.0f};
                glm::vec3 p11 = {x1, y1, 0.0f};
                glm::vec3 p01 = {x0, y1, 0.0f};

                glm::vec2 uv00 = {(float)x / xSubdivisions, (float)y / ySubdivisions};
                glm::vec2 uv10 = {(float)(x + 1) / xSubdivisions, (float)y / ySubdivisions};
                glm::vec2 uv11 = {(float)(x + 1) / xSubdivisions, (float)(y + 1) / ySubdivisions};
                glm::vec2 uv01 = {(float)x / xSubdivisions, (float)(y + 1) / ySubdivisions};

                // Triangle 1
                data.Positions.push_back(p00);
                data.Positions.push_back(p10);
                data.Positions.push_back(p11);

                data.TexCoords.push_back(uv00);
                data.TexCoords.push_back(uv10);
                data.TexCoords.push_back(uv11);

                data.Normals.push_back({0.0f, 0.0f, 1.0f});
                data.Normals.push_back({0.0f, 0.0f, 1.0f});
                data.Normals.push_back({0.0f, 0.0f, 1.0f});

                // Triangle 2
                data.Positions.push_back(p00);
                data.Positions.push_back(p11);
                data.Positions.push_back(p01);

                data.TexCoords.push_back(uv00);
                data.TexCoords.push_back(uv11);
                data.TexCoords.push_back(uv01);

                data.Normals.push_back({0.0f, 0.0f, 1.0f});
                data.Normals.push_back({0.0f, 0.0f, 1.0f});
                data.Normals.push_back({0.0f, 0.0f, 1.0f});
            }
        }

        ComputeSmoothNormals(data.Positions, data.Normals);

        auto mesh = CreateRef<Mesh>();
        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_FilePath = fmt::format("quad_{}x{}", xSubdivisions, ySubdivisions);
        return mesh;
    }

    Ref<Mesh> Mesh::CreateCube()
    {
        RawMeshData data;

        // Simple cube vertices (6 faces, 2 triangles per face)
        glm::vec3 positions[8] = {
            {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
            {-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, 0.5f, 0.5f},  {-0.5f, 0.5f, 0.5f},
        };

        uint32_t indices[36] = {
            0, 1, 2, 2, 3, 0, // back
            4, 5, 6, 6, 7, 4, // front
            4, 5, 1, 1, 0, 4, // bottom
            7, 6, 2, 2, 3, 7, // top
            4, 0, 3, 3, 7, 4, // left
            5, 1, 2, 2, 6, 5  // right
        };

        for (int i = 0; i < 36; i++)
        {
            data.Positions.push_back(positions[indices[i]]);
            // Simple normals based on face (approximation)
            data.Normals.push_back(glm::normalize(positions[indices[i]]));
            data.TexCoords.push_back({0.0f, 0.0f});
        }

        ComputeSmoothNormals(data.Positions, data.Normals);

        auto mesh = CreateRef<Mesh>();
        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_FilePath = "cube";
        return mesh;
    }

    Ref<Mesh> Mesh::Create(const std::string& filepath)
    {
        if (filepath == "quad")
            return CreateQuad();
        if (filepath == "cube")
            return CreateCube();

        Assimp::Importer importer;
        const aiScene* scene =
            importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace |
                                            aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality);

        if (!scene || !scene->mRootNode)
        {
            TI_CORE_ERROR("Failed to load mesh: {}", filepath);
            auto mesh = CreateRef<Mesh>();
            return mesh;
        }

        RawMeshData data;
        ProcessNode(scene->mRootNode, scene, data);

        ComputeSmoothNormals(data.Positions, data.Normals);

        auto mesh = CreateRef<Mesh>();
        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_FilePath = std::filesystem::relative(filepath).string();

        TI_CORE_INFO("Loaded mesh '{}' ({} vertices)", filepath, mesh->m_Positions.size());
        return mesh;
    }
} // namespace Titan
