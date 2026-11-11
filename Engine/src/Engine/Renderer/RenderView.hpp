#pragma once

#include <glm/glm.hpp>

class RenderView {

public:
    RenderView(
        const glm::mat4& view,
        const glm::mat4& projection,
        const glm::vec3& origin = {}
    ) :
        m_view{view},
        m_projection{projection},
        m_viewProjection{projection * view},
        m_origin{origin}
    {}

    auto view() const -> const auto& { return m_view; }
    auto projection() const -> const auto& { return m_projection; }
    auto viewProjection() const -> const auto& { return m_viewProjection; }
    auto origin() const -> const auto& { return m_origin; }

private:
    glm::mat4 m_view;
    glm::mat4 m_projection;
    glm::mat4 m_viewProjection;
    glm::vec3 m_origin;
};
