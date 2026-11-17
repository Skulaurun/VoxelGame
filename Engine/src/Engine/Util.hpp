#pragma once

#include <Engine/Core/Math.hpp>

#include <glm/glm.hpp>

#include <vector>

template<typename Vertex>
    requires std::is_standard_layout_v<Vertex> &&
    requires(Vertex v) {
        { v.position } -> std::same_as<glm::vec3&>;
        { v.normal }   -> std::same_as<glm::vec3&>;
        requires offsetof(Vertex, position) == 0;
        requires offsetof(Vertex, normal) == sizeof(glm::vec3);
    }
void generateSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::size_t resolution) {
    std::size_t resY = resolution;
    std::size_t resX = resolution;

    float theta = -90.0f;
    const float thetaDiff = 180.0f / resY;

    float phi = 0.0f;
    const float phiDiff = 360.0f / resX / 2.0f;

    indices.reserve(resX * resY * 6 * 2);
    vertices.reserve(resX * resY * 4);

    uint32_t j = 0;
    for (std::size_t y = 0; y < resY; y++) {
        uint32_t k = j;
        for (std::size_t x = 0; x < resX; x++) {
            glm::vec3 v0 = toCartesian(glm::radians(theta), glm::radians(phi));
            glm::vec3 v1 = toCartesian(glm::radians(theta), glm::radians(phi + phiDiff));
            glm::vec3 v2 = toCartesian(glm::radians(theta + thetaDiff), glm::radians(phi + phiDiff));
            glm::vec3 v3 = toCartesian(glm::radians(theta + thetaDiff), glm::radians(phi));

            vertices.emplace_back(v0, glm::normalize(v0));
            vertices.emplace_back(v1, glm::normalize(v1));
            vertices.emplace_back(v2, glm::normalize(v2));
            vertices.emplace_back(v3, glm::normalize(v3));

            /* Rectangle */
            indices.push_back(j + 0);
            indices.push_back(j + 1);
            indices.push_back(j + 2);
            
            indices.push_back(j + 0);
            indices.push_back(j + 2);
            indices.push_back(j + 3);

            if (x + 1 < resX) {
                /* The following rectangle */
                indices.push_back(j + 1);
                indices.push_back(j + 4);
                indices.push_back(j + 7);
            
                indices.push_back(j + 1);
                indices.push_back(j + 7);
                indices.push_back(j + 2);
            } else {
                /* Connect the following rectangle to the beginning */
                indices.push_back(j + 1);
                indices.push_back(k + 0);
                indices.push_back(k + 3);
            
                indices.push_back(j + 1);
                indices.push_back(k + 3);
                indices.push_back(j + 2);
            }

            phi += phiDiff * 2.0f;
            j += 4;
        }
        theta += thetaDiff;
    }
}
