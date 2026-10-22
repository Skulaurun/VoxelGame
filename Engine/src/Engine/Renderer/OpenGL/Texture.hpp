#pragma once

#include <cstdint>
#include <string_view>

class Texture {

public:
	Texture(uint32_t width, uint32_t height, uint32_t bytesPerPixel);
	Texture(std::string_view path);
	~Texture();

	void setData(uint8_t* data, uint32_t length);

	uint32_t width() const { return m_width; }
	uint32_t height() const { return m_height; }
	uint32_t bytesPerPixel() const { return m_bytesPerPixel; }

	void bind(uint32_t slot) const;

private:
	uint32_t m_id;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_bytesPerPixel;

	uint32_t m_internalFormat;

};
