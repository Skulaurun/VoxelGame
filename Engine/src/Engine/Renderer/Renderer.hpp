#pragma once

#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
    glm::vec2 textureCoordinate;
    float samplerID;
};

class VertexArray;

class Renderer {

public:
    static void init();

    static void clear(const glm::vec4& color = glm::vec4(0.0f));
    static void clear(const glm::vec3& color);

    static void draw(const VertexArray& vertexArray, uint32_t indexCount);

};
