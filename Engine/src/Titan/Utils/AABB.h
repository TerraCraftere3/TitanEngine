#pragma once

#include "Titan/PCH.h"

#undef min
#undef max

namespace Titan
{
    class TI_API AABB
    {
    public:
        glm::vec3 Min;
        glm::vec3 Max;

        AABB() noexcept;
        AABB(const glm::vec3& a, const glm::vec3& b) noexcept;

        static AABB FromCenterExtents(const glm::vec3& center, const glm::vec3& extents) noexcept;

        void Reset() noexcept;
        bool IsValid() const noexcept;

        glm::vec3 Center() const noexcept;
        glm::vec3 Extents() const noexcept;
        glm::vec3 Size() const noexcept;
        float Volume() const noexcept;

        void ExpandToInclude(const glm::vec3& p) noexcept;
        void ExpandToInclude(const AABB& other) noexcept;

        void Expand(float amount) noexcept;

        bool ContainsPoint(const glm::vec3& p) const noexcept;
        bool Contains(const AABB& other) const noexcept;

        bool Intersects(const AABB& other) const noexcept;
        AABB Intersection(const AABB& other) const noexcept;

        AABB Union(const AABB& other) const noexcept;

        void GetCorners(std::array<glm::vec3, 8>& outCorners) const noexcept;

        glm::vec3 ClosestPoint(const glm::vec3& p) const noexcept;
        float DistanceSquaredToPoint(const glm::vec3& p) const noexcept;

        bool IntersectRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float tMinLimit, float tMaxLimit,
                          float& outTNear, float& outTFar) const noexcept;

        AABB Transform(const glm::mat4& m) const noexcept;

        bool operator==(const AABB& o) const noexcept;
        bool operator!=(const AABB& o) const noexcept;
    };
} // namespace Titan
