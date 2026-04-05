#include <Engine/Renderer/OpenGL/VertexArray.hpp>

#include <glad/gl.h>

static GLenum getOpenGLType(VertexAttributeType type);

VertexArray::VertexArray() : vertexBufferIndex(0) {
	glCreateVertexArrays(1, &id);
}

VertexArray::~VertexArray() {
	glDeleteVertexArrays(1, &id);
}

void VertexArray::bind() const {
    glBindVertexArray(id);
}

void VertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
	glVertexArrayElementBuffer(id, indexBuffer->getId());
	this->indexBuffer = indexBuffer;
}

void VertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
	const auto& layout = vertexBuffer->getLayout();
	glVertexArrayVertexBuffer(id, vertexBufferIndex, vertexBuffer->getId(), 0, vertexBuffer->getStride());

	for (std::size_t i = 0; i < layout.size(); i++) {
        const auto& attribute = layout[i];

        switch (attribute.type) {
            case VertexAttributeType::Int:
            case VertexAttributeType::Int2:
            case VertexAttributeType::Int3:
            case VertexAttributeType::Int4:
            case VertexAttributeType::Bool:
                glVertexArrayAttribIFormat(
                    id, i,
                    attribute.getElementCount(),
                    getOpenGLType(attribute.type),
                    attribute.offset
                );
                break;
            case VertexAttributeType::Float:
            case VertexAttributeType::Float2:
            case VertexAttributeType::Float3:
            case VertexAttributeType::Float4:
                glVertexArrayAttribFormat(
                    id, i,
                    attribute.getElementCount(),
                    getOpenGLType(attribute.type),
                    attribute.normalized ? GL_TRUE : GL_FALSE,
                    attribute.offset
                );
                break;
        }
        glVertexArrayAttribBinding(id, i, vertexBufferIndex);
        glEnableVertexArrayAttrib(id, i);
    }
	vertexBuffers.emplace_back(vertexBuffer);
	vertexBufferIndex++;
}

static GLenum getOpenGLType(VertexAttributeType type) {
    switch (type) {
        case VertexAttributeType::Int:
        case VertexAttributeType::Int2:
        case VertexAttributeType::Int3:
        case VertexAttributeType::Int4:
            return GL_UNSIGNED_INT;
        case VertexAttributeType::Float:
        case VertexAttributeType::Float2:
        case VertexAttributeType::Float3:
        case VertexAttributeType::Float4:
            return GL_FLOAT;
        case VertexAttributeType::Bool:
            return GL_BOOL;
        default:
            return 0;
    }
}
