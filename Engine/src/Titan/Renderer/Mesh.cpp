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
        std::vector<glm::vec3> Tangents;
    };

    static void ProcessMesh(aiMesh* mesh, RawMeshData& data, std::vector<uint8_t>& materialIndexOut,
                            uint8_t materialIdx)
    {
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            if (face.mNumIndices != 3)
                continue;

            unsigned int idx0 = face.mIndices[0];
            unsigned int idx1 = face.mIndices[1];
            unsigned int idx2 = face.mIndices[2];

            glm::vec3 positions[3] = {
                {mesh->mVertices[idx0].x, mesh->mVertices[idx0].y, mesh->mVertices[idx0].z},
                {mesh->mVertices[idx1].x, mesh->mVertices[idx1].y, mesh->mVertices[idx1].z},
                {mesh->mVertices[idx2].x, mesh->mVertices[idx2].y, mesh->mVertices[idx2].z},
            };

            glm::vec2 uvs[3] = {
                mesh->HasTextureCoords(0) ? glm::vec2(mesh->mTextureCoords[0][idx0].x, mesh->mTextureCoords[0][idx0].y)
                                          : glm::vec2(0.0f),
                mesh->HasTextureCoords(0) ? glm::vec2(mesh->mTextureCoords[0][idx1].x, mesh->mTextureCoords[0][idx1].y)
                                          : glm::vec2(0.0f),
                mesh->HasTextureCoords(0) ? glm::vec2(mesh->mTextureCoords[0][idx2].x, mesh->mTextureCoords[0][idx2].y)
                                          : glm::vec2(0.0f)};

            glm::vec3 n[3] = {
                mesh->HasNormals() ? glm::vec3(mesh->mNormals[idx0].x, mesh->mNormals[idx0].y, mesh->mNormals[idx0].z)
                                   : glm::vec3(0.0f),
                mesh->HasNormals() ? glm::vec3(mesh->mNormals[idx1].x, mesh->mNormals[idx1].y, mesh->mNormals[idx1].z)
                                   : glm::vec3(0.0f),
                mesh->HasNormals() ? glm::vec3(mesh->mNormals[idx2].x, mesh->mNormals[idx2].y, mesh->mNormals[idx2].z)
                                   : glm::vec3(0.0f)};

            glm::vec3 edge1 = positions[1] - positions[0];
            glm::vec3 edge2 = positions[2] - positions[0];
            glm::vec2 deltaUV1 = uvs[1] - uvs[0];
            glm::vec2 deltaUV2 = uvs[2] - uvs[0];
            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);

            for (int j = 0; j < 3; ++j)
            {
                data.Positions.push_back(positions[j]);
                data.Normals.push_back(n[j]);
                data.TexCoords.push_back(uvs[j]);
                data.Tangents.push_back(tangent);
                materialIndexOut.push_back(materialIdx);
            }
        }
    }

    static void ProcessNode(aiNode* node, const aiScene* scene, RawMeshData& data,
                            std::vector<uint8_t>& materialIndexOut)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            uint8_t matIndex = static_cast<uint8_t>(mesh->mMaterialIndex);
            ProcessMesh(mesh, data, materialIndexOut, matIndex);
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
            ProcessNode(node->mChildren[i], scene, data, materialIndexOut);
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

    void ComputeTangents(const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& uvs,
                         const std::vector<uint32_t>& indices, std::vector<glm::vec3>& tangents)
    {
        tangents.resize(positions.size(), glm::vec3(0.0f));

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            uint32_t i0 = indices[i];
            uint32_t i1 = indices[i + 1];
            uint32_t i2 = indices[i + 2];

            const glm::vec3& p0 = positions[i0];
            const glm::vec3& p1 = positions[i1];
            const glm::vec3& p2 = positions[i2];

            const glm::vec2& uv0 = uvs[i0];
            const glm::vec2& uv1 = uvs[i1];
            const glm::vec2& uv2 = uvs[i2];

            glm::vec3 edge1 = p1 - p0;
            glm::vec3 edge2 = p2 - p0;

            glm::vec2 deltaUV1 = uv1 - uv0;
            glm::vec2 deltaUV2 = uv2 - uv0;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);

            tangents[i0] += tangent;
            tangents[i1] += tangent;
            tangents[i2] += tangent;
        }

        // Normalize
        for (auto& t : tangents)
            t = glm::normalize(t);
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

        // Tangent points along +X (U direction)
        data.Tangents = {
            {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f},
        };

        ComputeSmoothNormals(data.Positions, data.Normals);

        auto mesh = CreateRef<Mesh>();
        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_Tangents = std::move(data.Tangents);
        mesh->m_MaterialIndex = std::vector<uint8_t>(6, 0);
        mesh->m_Materials.push_back(CreateRef<Material3D>());
        mesh->m_FilePath = "quad";
        return mesh;
    }

    Ref<Mesh> Mesh::CreateCube()
    {
        RawMeshData data;

        // Cube vertices per face (6 faces, 2 triangles per face)
        struct Face
        {
            glm::vec3 normal;
            glm::vec3 tangent;
            glm::vec3 v0, v1, v2, v3;
        };

        Face faces[6] = {
            // Back
            {{0, 0, -1},
             {1, 0, 0},
             {-0.5f, -0.5f, -0.5f},
             {0.5f, -0.5f, -0.5f},
             {0.5f, 0.5f, -0.5f},
             {-0.5f, 0.5f, -0.5f}},
            // Front
            {{0, 0, 1}, {1, 0, 0}, {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}},
            // Bottom
            {{0, -1, 0},
             {1, 0, 0},
             {-0.5f, -0.5f, -0.5f},
             {0.5f, -0.5f, -0.5f},
             {0.5f, -0.5f, 0.5f},
             {-0.5f, -0.5f, 0.5f}},
            // Top
            {{0, 1, 0}, {1, 0, 0}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}},
            // Left
            {{-1, 0, 0},
             {0, 0, 1},
             {-0.5f, -0.5f, -0.5f},
             {-0.5f, 0.5f, -0.5f},
             {-0.5f, 0.5f, 0.5f},
             {-0.5f, -0.5f, 0.5f}},
            // Right
            {{1, 0, 0}, {0, 0, 1}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}},
        };

        glm::vec2 uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

        for (auto& f : faces)
        {
            // Triangle 1
            data.Positions.push_back(f.v0);
            data.TexCoords.push_back(uv[0]);
            data.Normals.push_back(f.normal);
            data.Tangents.push_back(f.tangent);
            data.Positions.push_back(f.v1);
            data.TexCoords.push_back(uv[1]);
            data.Normals.push_back(f.normal);
            data.Tangents.push_back(f.tangent);
            data.Positions.push_back(f.v2);
            data.TexCoords.push_back(uv[2]);
            data.Normals.push_back(f.normal);
            data.Tangents.push_back(f.tangent);
            // Triangle 2
            data.Positions.push_back(f.v0);
            data.TexCoords.push_back(uv[0]);
            data.Normals.push_back(f.normal);
            data.Tangents.push_back(f.tangent);
            data.Positions.push_back(f.v2);
            data.TexCoords.push_back(uv[2]);
            data.Normals.push_back(f.normal);
            data.Tangents.push_back(f.tangent);
            data.Positions.push_back(f.v3);
            data.TexCoords.push_back(uv[3]);
            data.Normals.push_back(f.normal);
            data.Tangents.push_back(f.tangent);
        }

        ComputeSmoothNormals(data.Positions, data.Normals);

        auto mesh = CreateRef<Mesh>();
        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_Tangents = std::move(data.Tangents);
        mesh->m_MaterialIndex = std::vector<uint8_t>(36, 0);
        mesh->m_Materials.push_back(CreateRef<Material3D>());
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

        auto mesh = CreateRef<Mesh>();
        if (!scene || !scene->mRootNode)
        {
            mesh->m_FilePath = filepath;
            return mesh;
        }

        RawMeshData data;
        std::vector<uint8_t> materialIndices;

        for (int i = 0; i < scene->mNumMaterials; i++)
        {
            mesh->m_Materials.push_back(CreateRef<Material3D>());
        }

        ProcessNode(scene->mRootNode, scene, data, materialIndices);
        ComputeSmoothNormals(data.Positions, data.Normals);

        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_Tangents = std::move(data.Tangents);
        mesh->m_MaterialIndex = std::move(materialIndices);

        mesh->m_FilePath = std::filesystem::relative(filepath).string();
        return mesh;
    }
} // namespace Titan
