#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>

#include <glad/gl.h>

VertexBuffer::VertexBuffer(uint32_t size)
    : VertexBuffer(nullptr, size) {}

VertexBuffer::VertexBuffer(const void* data, uint32_t size) : stride(0) {
    glCreateBuffers(1, &id);
    glNamedBufferData(id, size, data, data == nullptr ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &id);
}

void VertexBuffer::resize(const void* data, uint32_t size) {
    glNamedBufferData(id, size, data, GL_STATIC_DRAW);
}

void VertexBuffer::upload(const void* data, uint32_t offset, uint32_t size) {
    glNamedBufferSubData(id, offset, size, data);
}

void VertexBuffer::setLayout(const std::vector<VertexBufferAttribute>& layout) {
    attributeLayout = layout;
    stride = 0;

    for (auto& attribute : attributeLayout) {
        attribute.offset = stride;
        attribute.size = attribute.getElementCount() * attribute.getElementSize();

        stride += attribute.size;
    }
}

uint32_t VertexBufferAttribute::getElementSize() const {
    switch (type) {
        case VertexAttributeType::Int:
        case VertexAttributeType::Int2:
        case VertexAttributeType::Int3:
        case VertexAttributeType::Int4:
            return 4;
        case VertexAttributeType::Float:
        case VertexAttributeType::Float2:
        case VertexAttributeType::Float3:
        case VertexAttributeType::Float4:
            return 4;
        case VertexAttributeType::Bool:
            return 1;
        default:
            return 0;
    }
}

uint32_t VertexBufferAttribute::getElementCount() const {
    switch (type) {
        case VertexAttributeType::Int:
        case VertexAttributeType::Float:
        case VertexAttributeType::Bool:
            return 1;
        case VertexAttributeType::Int2:
        case VertexAttributeType::Float2:
            return 2;
        case VertexAttributeType::Int3:
        case VertexAttributeType::Float3:
            return 3;
        case VertexAttributeType::Int4:
        case VertexAttributeType::Float4:
            return 4;
        default:
            return 0;
    }
}
