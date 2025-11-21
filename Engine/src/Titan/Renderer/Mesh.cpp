#include "Mesh.h"
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <unordered_map>

namespace Titan
{
    struct RawMeshData
    {
        std::vector<glm::vec3> Positions;
        std::vector<glm::vec3> Normals;
        std::vector<glm::vec2> TexCoords;
        std::vector<glm::vec3> Tangents;
        std::vector<uint32_t> Indices;
    };

    struct PackedVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 UV;
        glm::vec3 Tangent;

        bool operator==(const PackedVertex& other) const
        {
            return Position == other.Position && Normal == other.Normal && UV == other.UV && Tangent == other.Tangent;
        }
    };

    struct PackedVertexHash
    {
        size_t operator()(const PackedVertex& v) const
        {
            size_t h1 = std::hash<float>()(v.Position.x) ^ (std::hash<float>()(v.Position.y) << 1) ^
                        (std::hash<float>()(v.Position.z) << 2);
            size_t h2 = std::hash<float>()(v.Normal.x) ^ (std::hash<float>()(v.Normal.y) << 1) ^
                        (std::hash<float>()(v.Normal.z) << 2);
            size_t h3 = std::hash<float>()(v.UV.x) ^ (std::hash<float>()(v.UV.y) << 1);
            size_t h4 = std::hash<float>()(v.Tangent.x) ^ (std::hash<float>()(v.Tangent.y) << 1) ^
                        (std::hash<float>()(v.Tangent.z) << 2);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };

    static glm::mat4 ConvertMatrix(const aiMatrix4x4& m)
    {
        glm::mat4 r;
        r[0][0] = m.a1;
        r[1][0] = m.a2;
        r[2][0] = m.a3;
        r[3][0] = m.a4;
        r[0][1] = m.b1;
        r[1][1] = m.b2;
        r[2][1] = m.b3;
        r[3][1] = m.b4;
        r[0][2] = m.c1;
        r[1][2] = m.c2;
        r[2][2] = m.c3;
        r[3][2] = m.c4;
        r[0][3] = m.d1;
        r[1][3] = m.d2;
        r[2][3] = m.d3;
        r[3][3] = m.d4;
        return r;
    }

    static void ProcessMesh(aiMesh* mesh, RawMeshData& data, std::vector<uint8_t>& materialIndexOut,
                            uint8_t materialIdx, const glm::mat4& transform)
    {
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
        std::unordered_map<PackedVertex, uint32_t, PackedVertexHash> vertexToIndex;

        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            if (face.mNumIndices != 3)
                continue;

            for (int j = 0; j < 3; j++)
            {
                unsigned int idx = face.mIndices[j];

                glm::vec3 pos = {mesh->mVertices[idx].x, mesh->mVertices[idx].y, mesh->mVertices[idx].z};
                glm::vec2 uv = mesh->HasTextureCoords(0)
                                   ? glm::vec2(mesh->mTextureCoords[0][idx].x, mesh->mTextureCoords[0][idx].y)
                                   : glm::vec2(0.0f);
                glm::vec3 normal = mesh->HasNormals()
                                       ? glm::vec3(mesh->mNormals[idx].x, mesh->mNormals[idx].y, mesh->mNormals[idx].z)
                                       : glm::vec3(0.0f);

                pos = glm::vec3(transform * glm::vec4(pos, 1.0f));
                normal = glm::normalize(normalMatrix * normal);

                // Tangent calculation (simplified; replace with your deltaUV method if needed)
                glm::vec3 tangent = glm::vec3(1, 0, 0);

                PackedVertex v{pos, normal, uv, tangent};

                uint32_t finalIndex;
                auto it = vertexToIndex.find(v);
                if (it == vertexToIndex.end())
                {
                    finalIndex = static_cast<uint32_t>(data.Positions.size());
                    vertexToIndex[v] = finalIndex;

                    data.Positions.push_back(pos);
                    data.Normals.push_back(normal);
                    data.TexCoords.push_back(uv);
                    data.Tangents.push_back(tangent);
                    materialIndexOut.push_back(materialIdx);
                }
                else
                    finalIndex = it->second;

                data.Indices.push_back(finalIndex);
            }
        }
    }

    static void ProcessNode(aiNode* node, const aiScene* scene, RawMeshData& data,
                            std::vector<uint8_t>& materialIndexOut, const glm::mat4& parentTransform)
    {
        glm::mat4 nodeTransform = parentTransform * ConvertMatrix(node->mTransformation);

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            uint8_t matIndex = static_cast<uint8_t>(mesh->mMaterialIndex);
            ProcessMesh(mesh, data, materialIndexOut, matIndex, nodeTransform);
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
            ProcessNode(node->mChildren[i], scene, data, materialIndexOut, nodeTransform);
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
        std::vector<uint8_t> matIndices;

        // Quad on XY plane
        glm::vec3 positions[4] = {{-0.5f, -0.5f, 0}, {0.5f, -0.5f, 0}, {0.5f, 0.5f, 0}, {-0.5f, 0.5f, 0}};
        glm::vec2 uvs[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        glm::vec3 normal = {0, 0, 1};
        glm::vec3 tangent = {1, 0, 0};

        uint8_t materialIdx = 0;
        auto addVertex = [&](int i)
        {
            data.Positions.push_back(positions[i]);
            data.Normals.push_back(normal);
            data.TexCoords.push_back(uvs[i]);
            data.Tangents.push_back(tangent);
            matIndices.push_back(materialIdx);
        };

        // Two triangles
        addVertex(0);
        addVertex(1);
        addVertex(2);
        data.Indices.push_back(0);
        data.Indices.push_back(1);
        data.Indices.push_back(2);
        addVertex(0);
        addVertex(2);
        addVertex(3);
        data.Indices.push_back(3);
        data.Indices.push_back(4);
        data.Indices.push_back(5);

        auto mesh = CreateRef<Mesh>();
        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_Tangents = std::move(data.Tangents);
        mesh->m_Indices = std::move(data.Indices);
        mesh->m_MaterialIndex = std::move(matIndices);

        auto material = CreateRef<Material3D>();
        material->Name = "Material";
        mesh->m_Materials.push_back(material);
        mesh->m_FilePath = "quad";
        mesh->ComputeBounds();
        return mesh;
    }

    Ref<Mesh> Mesh::CreateCube()
    {
        RawMeshData data;
        std::vector<uint8_t> matIndices;

        struct Face
        {
            glm::vec3 normal, tangent;
            glm::vec3 v0, v1, v2, v3;
        };
        Face faces[6] = {
            {{0, 0, -1},
             {1, 0, 0},
             {-0.5f, -0.5f, -0.5f},
             {0.5f, -0.5f, -0.5f},
             {0.5f, 0.5f, -0.5f},
             {-0.5f, 0.5f, -0.5f}},
            {{0, 0, 1}, {1, 0, 0}, {-0.5f, -0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}},
            {{0, -1, 0},
             {1, 0, 0},
             {-0.5f, -0.5f, -0.5f},
             {0.5f, -0.5f, -0.5f},
             {0.5f, -0.5f, 0.5f},
             {-0.5f, -0.5f, 0.5f}},
            {{0, 1, 0}, {1, 0, 0}, {-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}},
            {{-1, 0, 0},
             {0, 0, 1},
             {-0.5f, -0.5f, -0.5f},
             {-0.5f, 0.5f, -0.5f},
             {-0.5f, 0.5f, 0.5f},
             {-0.5f, -0.5f, 0.5f}},
            {{1, 0, 0}, {0, 0, 1}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {0.5f, -0.5f, 0.5f}},
        };
        glm::vec2 uv[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        uint8_t materialIdx = 0;
        for (auto& f : faces)
        {
            uint32_t start = (uint32_t)data.Positions.size();
            data.Positions.insert(data.Positions.end(), {f.v0, f.v1, f.v2, f.v3});
            data.Normals.insert(data.Normals.end(), 4, f.normal);
            data.Tangents.insert(data.Tangents.end(), 4, f.tangent);
            data.TexCoords.insert(data.TexCoords.end(), {uv[0], uv[1], uv[2], uv[3]});
            matIndices.insert(matIndices.end(), 4, materialIdx);

            data.Indices.insert(data.Indices.end(), {start, start + 1, start + 2, start, start + 2, start + 3});
        }

        auto mesh = CreateRef<Mesh>();
        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_Tangents = std::move(data.Tangents);
        mesh->m_Indices = std::move(data.Indices);
        mesh->m_MaterialIndex = std::move(matIndices);

        auto material = CreateRef<Material3D>();
        material->Name = "Material";
        mesh->m_Materials.push_back(material);
        mesh->m_FilePath = "cube";
        mesh->ComputeBounds();
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

        // Load materials
        for (unsigned int i = 0; i < scene->mNumMaterials; i++)
        {
            auto material = CreateRef<Material3D>();
            aiString aiMatName;
            if (scene->mMaterials[i]->Get(AI_MATKEY_NAME, aiMatName) == AI_SUCCESS && aiMatName.length > 0)
                material->Name = aiMatName.C_Str();
            else
                material->Name = "Material " + std::to_string(i + 1);
            mesh->m_Materials.push_back(material);
        }

        ProcessNode(scene->mRootNode, scene, data, materialIndices, glm::mat4(1.0f));

        ComputeSmoothNormals(data.Positions, data.Normals);

        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);
        mesh->m_Tangents = std::move(data.Tangents);
        mesh->m_Indices = std::move(data.Indices);
        mesh->m_MaterialIndex = std::move(materialIndices);

        mesh->m_FilePath = std::filesystem::relative(filepath).string();
        mesh->ComputeBounds();
        return mesh;
    }

    void Mesh::ComputeBounds()
    {
        m_Bounds.Reset();

        for (const auto& p : m_Positions)
            m_Bounds.ExpandToInclude(p);
    }
} // namespace Titan
