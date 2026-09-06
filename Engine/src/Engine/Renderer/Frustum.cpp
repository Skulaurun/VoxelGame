#include <Engine/Renderer/Frustum.hpp>

float Frustum::distanceToPoint(const Plane& plane, const Point3& point) {
    return glm::dot(plane.normal, point) + plane.constant;
}

bool Frustum::intersectsBox(const Box3& box) const {
    for (std::size_t i = 0; i < m_planes.size(); i++) {
        const Plane& plane = m_planes[i];
        // TODO: learn
        glm::vec3 point{
            plane.normal.x > 0 ? box.max.x : box.min.x,
            plane.normal.y > 0 ? box.max.y : box.min.y,
            plane.normal.z > 0 ? box.max.z : box.min.z
        };
        if (distanceToPoint(plane, point) < 0) {
            return false;
        }
    }

    return true;
}

bool Frustum::intersectsPoint(const Point3& point) const {
    for (std::size_t i = 0; i < m_planes.size(); i++) {
        if (distanceToPoint(m_planes[i], point) < 0) {
            return false;
        }
    }
    return true;
}

void Frustum::update(const glm::mat4& viewProjection) {
    glm::mat4 m = glm::transpose(viewProjection);

    // https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
    std::array<glm::vec4, 6> planes{};
    // left
    planes[0] = m[3] + m[0];
    // right
    planes[1] = m[3] - m[0];
    // bottom
    planes[2] = m[3] + m[1];
    // top
    planes[3] = m[3] - m[1];
    // near
    planes[4] = m[3] + m[2];
    // far
    planes[5] = m[3] - m[2];

    for (std::size_t i = 0; i < planes.size(); i++) {
        // normalize
        planes[i] /= glm::length(glm::vec3{planes[i]});

        m_planes[i] = Plane{ glm::vec3{planes[i]}, planes[i].w };
    }
}
