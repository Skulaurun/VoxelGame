#pragma once

#include <Engine/Core/Core.hpp>

#include <glm/glm.hpp>

// TODO: Recalculate in setters

class PerspectiveCamera {

public:
    PerspectiveCamera(float fovY, float aspectRatio, float near, float far);

    void setFovY(float fovY) { m_fovY = fovY; m_projectionDirty = true; }
    void setNearPlane(float near) { m_near = near; m_projectionDirty = true; }
    void setFarPlane(float far) { m_far = far; m_projectionDirty = true; }
    void setAspectRatio(float aspectRatio) { m_aspectRatio = aspectRatio; m_projectionDirty = true; }

    void setPosition(const WorldPosition& position) { m_position = position; m_viewDirty = true; }
    void setDirection(const glm::vec3& direction) { m_direction = direction; m_viewDirty = true; }
    
    auto fovY() const { return m_fovY; }
    auto position() const { return m_position; }
    auto direction() const { return m_direction; }

    auto view() const -> const auto& {
        if (m_viewDirty) {
            updateView();
            m_viewDirty = false;
        }
        return m_view;
    }
    auto projection() const -> const auto& {
        if (m_projectionDirty) {
            updateProjection();
            m_projectionDirty = false;
        }
        return m_projection;
    }
    auto viewProjection() const -> const auto& {
        if (m_viewDirty || m_projectionDirty) {
            if (m_viewDirty) updateView();
            if (m_projectionDirty) updateProjection();
            updateViewProjection();
            m_viewDirty = m_projectionDirty = false;
        }
        return m_viewProjection;
    }

private:
    void updateView() const;
    void updateProjection() const;
    void updateViewProjection() const;

private:
    float m_fovY;
    float m_aspectRatio;
    float m_near;
    float m_far;

    WorldPosition m_position;
    WorldPosition m_direction;

    mutable bool m_viewDirty;
    mutable bool m_projectionDirty;

    mutable glm::mat4 m_view;
    mutable glm::mat4 m_projection;
    mutable glm::mat4 m_viewProjection;

};
