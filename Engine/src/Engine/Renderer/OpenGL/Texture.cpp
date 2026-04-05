#include <Engine/Renderer/OpenGL/Texture.hpp>

#include <glad/gl.h>
#include <stb_image.h>

GLenum baseFormatFromInternalFormat(GLenum internalFormat) {
    switch (internalFormat) {
        case GL_RGB8: return GL_RGB;
        case GL_RGBA8: return GL_RGBA;
        default: return GL_NONE;
    }
}
GLenum internalFormatFromBPP(uint32_t bytesPerPixel) {
    switch (bytesPerPixel) {
        case 4: return GL_RGBA8;
        case 3: return GL_RGB8;
        default: return GL_NONE;
    }
}

Texture::Texture(uint32_t width, uint32_t height, uint32_t bytesPerPixel)
    : id(0), width(width), height(height), bytesPerPixel(bytesPerPixel) {

    glCreateTextures(GL_TEXTURE_2D, 1, &id);
    internalFormat = internalFormatFromBPP(bytesPerPixel);
    glTextureStorage2D(id, 1, internalFormat, width, height);

    glTexParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
Texture::Texture(std::string_view path) {
    int w, h, n;
    uint8_t* data = stbi_load(path.data(), &w, &h, &n, 0);
    if (data) {
        width = w;
        height = h;
        bytesPerPixel = n;

        glCreateTextures(GL_TEXTURE_2D, 1, &id);
        internalFormat = internalFormatFromBPP(bytesPerPixel);
        glTextureStorage2D(id, 1, internalFormat, width, height);

        glTexParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureSubImage2D(id, 0, 0, 0, width, height, baseFormatFromInternalFormat(internalFormat), GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }
}
Texture::~Texture() {
    if (id) {
        glDeleteTextures(1, &id);
    }
}

void Texture::setData(uint8_t* data, uint32_t length) {
    glTextureSubImage2D(id, 0, 0, 0, width, height, baseFormatFromInternalFormat(internalFormat), GL_UNSIGNED_BYTE, data);
}

void Texture::bind(const uint32_t slot) const {
    glBindTextureUnit(slot, id);
}
