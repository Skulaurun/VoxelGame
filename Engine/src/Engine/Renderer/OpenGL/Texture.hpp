#pragma once

#include <cstdint>
#include <string_view>

class Texture {

public:
	Texture(uint32_t width, uint32_t height, uint32_t bytesPerPixel);
	Texture(std::string_view path);
	~Texture();

	void setData(uint8_t* data, uint32_t length);

	uint32_t getWidth() const { return width; }
	uint32_t getHeight() const { return height; }
	uint32_t getBytesPerPixel() const { return bytesPerPixel; }

	void bind(uint32_t slot) const;

private:
	uint32_t id;
	uint32_t width;
	uint32_t height;
	uint32_t bytesPerPixel;

	uint32_t internalFormat;

};
