#pragma once

#include <cstdint>

class IndexBuffer {

public:
	IndexBuffer(uint32_t size);
	IndexBuffer(const uint32_t* data, uint32_t size);
	~IndexBuffer();

	void resize(const void* data, uint32_t size);

	void upload(const uint32_t* data, uint32_t offset, uint32_t size);

	uint32_t getId() const { return id; }

private:
	uint32_t id;

};
