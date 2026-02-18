#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Titan/Core.h"

namespace Titan
{
    struct QuadMeshVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec2 TexCoord;
    };

    struct QuadMeshData
    {
        std::vector<QuadMeshVertex> Vertices;
        std::vector<uint32_t> Indices;
    };

    class QuadMeshGenerator
    {
    public:
        /**
         * Generate a quad mesh with specified vertex density
         * @param verticesX Number of vertices along the X axis
         * @param verticesZ Number of vertices along the Z axis
         * @param width Width of the quad (in X direction)
         * @param height Height of the quad (in Z direction)
         * @return QuadMeshData containing vertices and indices
         */
        static QuadMeshData GenerateQuad(uint32_t verticesX, uint32_t verticesZ, float width = 1.0f,
                                         float height = 1.0f);
    };
} // namespace Titan
