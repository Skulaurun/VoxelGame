#include <Engine/Renderer/PerspectiveCamera.hpp>

#include <glm/gtc/matrix_transform.hpp>

PerspectiveCamera::PerspectiveCamera(float fovY, float aspectRatio, float near, float far) :
    m_fovY{fovY},
    m_aspectRatio{aspectRatio},
    m_near{near},
    m_far{far},

    m_position{},
    m_direction{0.0f, 0.0f, -1.0f},
    
    m_viewDirty{true},
    m_projectionDirty{true},

    m_view{1.0f},
    m_projection{1.0f}
{
    updateView();
    updateProjection();
}

void PerspectiveCamera::updateView() const {
    m_view = glm::lookAt(m_position, m_position + m_direction, WORLD_UP);
}

void PerspectiveCamera::updateProjection() const {
    m_projection = glm::perspective(glm::radians(m_fovY), m_aspectRatio, m_near, m_far);
}
