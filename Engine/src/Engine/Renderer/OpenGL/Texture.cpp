#include <Engine/Renderer/OpenGL/Texture.hpp>

#include <glad/gl.h>
#include <stb_image.h>

GLenum baseFormatFromInternalFormat(GLenum internalFormat) {
    switch (internalFormat) {
        case GL_RGB8: return GL_RGB;
        case GL_RGBA8: return GL_RGBA;
        case GL_RGBA16F: return GL_RGBA;
        case GL_DEPTH_COMPONENT24: return GL_DEPTH_COMPONENT;
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
    : m_id(0), m_width(width), m_height(height), m_bytesPerPixel(bytesPerPixel) {

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    m_internalFormat = internalFormatFromBPP(bytesPerPixel);
    glTextureStorage2D(m_id, 1, m_internalFormat, m_width, m_height);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::Texture(std::string_view path) {
    int w, h, n;
    uint8_t* data = stbi_load(path.data(), &w, &h, &n, 0);
    if (data) {
        m_width = w;
        m_height = h;
        m_bytesPerPixel = n;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
        m_internalFormat = internalFormatFromBPP(m_bytesPerPixel);
        glTextureStorage2D(m_id, 1, m_internalFormat, m_width, m_height);

        glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, baseFormatFromInternalFormat(m_internalFormat), GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }
}

Texture::~Texture() {
    if (m_id) {
        glDeleteTextures(1, &m_id);
    }
}

void Texture::setData(uint8_t* data, uint32_t length) {
    glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, baseFormatFromInternalFormat(m_internalFormat), GL_UNSIGNED_BYTE, data);
}

void Texture::bind(uint32_t slot) const {
    glBindTextureUnit(slot, m_id);
}
