#pragma once

#include <Engine/Core/Math.hpp>

#include <array>

class Frustum {

public:
    bool intersectsBox(const Box3& box) const;
    bool intersectsPoint(const Point3& point) const;

    void update(const glm::mat4& viewProjection);

private:
    static float distanceToPoint(const Plane& plane, const Point3& point);

private:
    std::array<Plane, 6> m_planes;

};
