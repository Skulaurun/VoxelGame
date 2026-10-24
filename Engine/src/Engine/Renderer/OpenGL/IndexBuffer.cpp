#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>

#include <glad/gl.h>

IndexBuffer::IndexBuffer(uint32_t size)
    : IndexBuffer(nullptr, size) {}

IndexBuffer::IndexBuffer(const uint32_t* data, const uint32_t size) {
    glCreateBuffers(1, &id);
    glNamedBufferData(id, size, data, data == nullptr ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer() {
    glDeleteBuffers(1, &id);
}

void IndexBuffer::resize(const void* data, uint32_t size) {
    glNamedBufferData(id, size, data, GL_STATIC_DRAW);
}

void IndexBuffer::upload(const uint32_t* data, uint32_t offset, uint32_t size) {
    glNamedBufferSubData(id, offset, size, data);
}
