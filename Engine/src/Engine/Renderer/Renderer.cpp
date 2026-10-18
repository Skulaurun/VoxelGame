#include <Engine/Renderer/Renderer.hpp>

#include <Engine/Renderer/OpenGL/OpenGL.hpp>
#include <Engine/Renderer/OpenGL/Shader.hpp>
#include <Engine/Renderer/OpenGL/Texture.hpp>
#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>

#include <glad/gl.h>

void Renderer::init() {
    OpenGL::init();
}

void Renderer::clear(const glm::vec4& color) {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::clear(const glm::vec3& color) {
    glClearColor(color.x, color.y, color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::draw(const VertexArray& vertexArray, uint32_t indexCount) {
    vertexArray.bind();
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

void Renderer::setViewport(const glm::ivec2& size, const glm::ivec2& lowerLeftCorner) {
    glViewport(lowerLeftCorner.x, lowerLeftCorner.y, size.x, size.y);
}
