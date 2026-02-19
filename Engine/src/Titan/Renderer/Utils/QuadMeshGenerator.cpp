#include "QuadMeshGenerator.h"
#include <glm/geometric.hpp>

namespace Titan
{
    QuadMeshData QuadMeshGenerator::GenerateQuad(uint32_t verticesX, uint32_t verticesZ, float width, float height,
                                                 glm::vec2 UV0, glm::vec2 UV1)
    {
        QuadMeshData meshData;

        // Generate vertices
        for (uint32_t z = 0; z < verticesZ; ++z)
        {
            for (uint32_t x = 0; x < verticesX; ++x)
            {
                QuadMeshVertex vertex;

                // Normalize coordinates to [0, 1]
                float u = (verticesX > 1) ? static_cast<float>(x) / (verticesX - 1) : 0.0f;
                float v = (verticesZ > 1) ? static_cast<float>(z) / (verticesZ - 1) : 0.0f;

                // Position
                vertex.Position = glm::vec3(u * width - width * 0.5f, 0.0f, v * height - height * 0.5f);

                // Normal (pointing up for a flat plane)
                vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);

                // Tangent (along the X axis)
                vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);

                // Texture coordinates
                vertex.TexCoord = glm::vec2(UV0.x + u * (UV1.x - UV0.x), UV0.y + v * (UV1.y - UV0.y));

                meshData.Vertices.push_back(vertex);
            }
        }

        // Generate indices (two triangles per quad cell)
        for (uint32_t z = 0; z < verticesZ - 1; ++z)
        {
            for (uint32_t x = 0; x < verticesX - 1; ++x)
            {
                // Current vertex indices
                uint32_t topLeft = z * verticesX + x;
                uint32_t topRight = topLeft + 1;
                uint32_t bottomLeft = topLeft + verticesX;
                uint32_t bottomRight = bottomLeft + 1;

                // First triangle (top-left, bottom-left, top-right)
                meshData.Indices.push_back(topLeft);
                meshData.Indices.push_back(bottomLeft);
                meshData.Indices.push_back(topRight);

                // Second triangle (top-right, bottom-left, bottom-right)
                meshData.Indices.push_back(topRight);
                meshData.Indices.push_back(bottomLeft);
                meshData.Indices.push_back(bottomRight);
            }
        }

        return meshData;
    }
} // namespace Titan
