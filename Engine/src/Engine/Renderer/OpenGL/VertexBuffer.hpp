#pragma once

#include <vector>
#include <cstdint>

enum class VertexAttributeType : uint8_t {
	Bool,
	Int, Int2, Int3, Int4,
	Float, Float2, Float3, Float4
};

struct VertexBufferAttribute {
	VertexAttributeType type;
	bool normalized;

	uint32_t size;
	uint32_t offset;

	VertexBufferAttribute(const VertexAttributeType type, const bool normalized = false)
		: type(type), normalized(normalized), size(0), offset(0) {}

	uint32_t getElementCount() const;
	uint32_t getElementSize() const;
};

class VertexBuffer {

public:
	VertexBuffer(uint32_t size);
	VertexBuffer(const void* data, uint32_t size);
	~VertexBuffer();

	void resize(const void* data, uint32_t size);

	void upload(const void* data, uint32_t offset, uint32_t size);

	uint32_t getId() const { return id; }
	uint32_t getStride() const { return stride; }

	void setLayout(const std::vector<VertexBufferAttribute>& layout);
	const std::vector<VertexBufferAttribute>& getLayout() const { return attributeLayout; }

private:
	uint32_t id;
	uint32_t stride;
	std::vector<VertexBufferAttribute> attributeLayout;

};
