#pragma once

#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>

#include <memory>
#include <vector>

class VertexArray {

public:
	VertexArray();
	virtual ~VertexArray();

	void bind() const;

	void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);
	void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer);

	const std::shared_ptr<IndexBuffer>& getIndexBuffer() const { return indexBuffer; }
	const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const { return vertexBuffers; }

private:
	uint32_t id;
	uint32_t vertexBufferIndex;

	std::shared_ptr<IndexBuffer> indexBuffer;
	std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;

};
