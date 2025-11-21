#include "AABB.h"
#include <limits>
#include <algorithm>

namespace Titan{

    // Construct an invalid (empty) AABB by default
    AABB::AABB() noexcept
        : Min(std::numeric_limits<float>::infinity()), Max(-std::numeric_limits<float>::infinity()){}

    // Construct from min/max (will be normalized so Min <= Max component-wise)
    AABB::AABB(const glm::vec3& a, const glm::vec3& b) noexcept
    {
        Min = glm::min(a, b);
        Max = glm::max(a, b);
    }

    AABB AABB::FromCenterExtents(const glm::vec3& center, const glm::vec3& extents) noexcept{
        return AABB(center - extents, center + extents);
    }

    // Reset to invalid box
    void AABB::Reset() noexcept{
        Min = glm::vec3(std::numeric_limits<float>::infinity());
        Max = glm::vec3(-std::numeric_limits<float>::infinity());
    }

    bool AABB::IsValid() const noexcept{
        return (Min.x <= Max.x) && (Min.y <= Max.y) && (Min.z <= Max.z);
    }

    glm::vec3 AABB::Center() const noexcept{
        return (Min + Max) * 0.5f;
    }

    glm::vec3 AABB::Extents() const noexcept{ // half-sizes
        return (Max - Min) * 0.5f;
    }

    glm::vec3 AABB::Size() const noexcept{
        return Max - Min;
    }

    float AABB::Volume() const noexcept{
        glm::vec3 s = Size();
        return s.x * s.y * s.z;
    }

    // Expand to include a point
    void AABB::ExpandToInclude(const glm::vec3& p) noexcept{
        Min = glm::min(Min, p);
        Max = glm::max(Max, p);
    }

    // Expand to include another AABB
    void AABB::ExpandToInclude(const AABB& other) noexcept{
        if(!other.IsValid()) return;
        Min = glm::min(Min, other.Min);
        Max = glm::max(Max, other.Max);
    }

    // Expand each axis by constant (symmetric)
    void AABB::Expand(float amount) noexcept{
        glm::vec3 a(amount);
        Min -= a;
        Max += a;
    }

    // Returns true if this contains point (including boundary)
    bool AABB::ContainsPoint(const glm::vec3& p) const noexcept{
        if(!IsValid()) return false;
        return (p.x >= Min.x && p.x <= Max.x)
            && (p.y >= Min.y && p.y <= Max.y)
            && (p.z >= Min.z && p.z <= Max.z);
    }

    // Contains AABB (true if other is fully inside this)
    bool AABB::Contains(const AABB& other) const noexcept{
        if(!IsValid() || !other.IsValid()) return false;
        return (other.Min.x >= Min.x && other.Max.x <= Max.x)
            && (other.Min.y >= Min.y && other.Max.y <= Max.y)
            && (other.Min.z >= Min.z && other.Max.z <= Max.z);
    }

    // Intersects other (true if overlapping or touching)
    bool AABB::Intersects(const AABB& other) const noexcept{
        if(!IsValid() || !other.IsValid()) return false;
        return (Min.x <= other.Max.x && Max.x >= other.Min.x)
            && (Min.y <= other.Max.y && Max.y >= other.Min.y)
            && (Min.z <= other.Max.z && Max.z >= other.Min.z);
    }

    // Compute intersection AABB; returns invalid AABB if they don't intersect
    AABB AABB::Intersection(const AABB& other) const noexcept{
        if(!Intersects(other)) return AABB();
        glm::vec3 mi = glm::max(Min, other.Min);
        glm::vec3 ma = glm::min(Max, other.Max);
        return AABB(mi, ma);
    }

    // Union AABB (smallest AABB containing both)
    AABB AABB::Union(const AABB& other) const noexcept{
        if(!IsValid()) return other;
        if(!other.IsValid()) return *this;
        return AABB(glm::min(Min, other.Min), glm::max(Max, other.Max));
    }

    // Get 8 corners in a deterministic order
    void AABB::GetCorners(std::array<glm::vec3,8>& outCorners) const noexcept{
        // order: (x,y,z) with Min/Max chooses
        outCorners[0] = glm::vec3(Min.x, Min.y, Min.z);
        outCorners[1] = glm::vec3(Max.x, Min.y, Min.z);
        outCorners[2] = glm::vec3(Min.x, Max.y, Min.z);
        outCorners[3] = glm::vec3(Max.x, Max.y, Min.z);
        outCorners[4] = glm::vec3(Min.x, Min.y, Max.z);
        outCorners[5] = glm::vec3(Max.x, Min.y, Max.z);
        outCorners[6] = glm::vec3(Min.x, Max.y, Max.z);
        outCorners[7] = glm::vec3(Max.x, Max.y, Max.z);
    }

    // Closest point on/in the AABB to a given point
    glm::vec3 AABB::ClosestPoint(const glm::vec3& p) const noexcept{
        if(!IsValid()) return p;
        return glm::clamp(p, Min, Max);
    }

    float AABB::DistanceSquaredToPoint(const glm::vec3& p) const noexcept{
        glm::vec3 c = ClosestPoint(p);
        glm::vec3 d = c - p;
        return glm::dot(d, d);
    }

    // Ray-AABB intersection (slab method)
    // rayDir must be non-zero vector; returns true if intersects within [tMinLimit, tMaxLimit]
    // on success, outTNear and outTFar will be filled with the entering/exiting t values
    bool AABB::IntersectRay(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float tMinLimit, float tMaxLimit, float& outTNear, float& outTFar) const noexcept{
        outTNear = tMinLimit;
        outTFar  = tMaxLimit;
        if(!IsValid()) return false;

        for(int i=0;i<3;++i){
            float origin = rayOrigin[i];
            float dir = rayDir[i];
            float invD;
            if(std::abs(dir) < 1e-8f){
                // Ray is parallel to slab; no hit if origin not within slab
                if(origin < Min[i] || origin > Max[i]) return false;
                else continue;
            }
            invD = 1.0f / dir;
            float t0 = (Min[i] - origin) * invD;
            float t1 = (Max[i] - origin) * invD;
            if(t0 > t1) std::swap(t0, t1);
            outTNear = std::max(outTNear, t0);
            outTFar  = std::min(outTFar,  t1);
            if(outTNear > outTFar) return false;
        }
        return true;
    }

    // Transform the AABB by an arbitrary 4x4 matrix and return the axis-aligned bounding box of the transformed box
    AABB AABB::Transform(const glm::mat4& m) const noexcept{
        if(!IsValid()) return AABB();
        std::array<glm::vec3,8> c;
        GetCorners(c);
        AABB r;
        for(const auto& corner : c){
            glm::vec4 tc = m * glm::vec4(corner, 1.0f);
            r.ExpandToInclude(glm::vec3(tc) / tc.w);
        }
        return r;
    }

    // Operators
    bool AABB::operator==(const AABB& o) const noexcept{
        return Min == o.Min && Max == o.Max;
    }
    bool AABB::operator!=(const AABB& o) const noexcept{ return !(*this == o); }

} // namespace Titan
