#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

    Ref<Mesh> Mesh::Create(const std::string& filepath)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filepath,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ImproveCacheLocality
        );

        if (!scene || !scene->mRootNode)
        {
            TI_CORE_ERROR("Failed to load mesh: {}", filepath);
            return nullptr;
        }

        RawMeshData data;
        ProcessNode(scene->mRootNode, scene, data);

        auto mesh = CreateRef<Mesh>();
        mesh->m_Positions = std::move(data.Positions);
        mesh->m_Normals   = std::move(data.Normals);
        mesh->m_TexCoords = std::move(data.TexCoords);

        TI_CORE_INFO("Loaded mesh '{}' ({} vertices)", filepath, mesh->m_Positions.size());
        return mesh;
    }
}
