#include <Engine/Renderer/OpenGL/FrameBuffer.hpp>
#include <Engine/Renderer/OpenGL/Texture.hpp>

#include <glad/gl.h>

#include <cassert>

auto pixelFormat(PixelType type) -> GLenum {
    switch (type) {
        case PixelType::Color8: return GL_RGBA8;
        case PixelType::Float16: return GL_RGBA16F;
        case PixelType::Depth24: return GL_DEPTH_COMPONENT24;
        default: return GL_NONE;
    }
}

FrameBuffer::FrameBuffer(const Specification& specification) : m_specification{specification} {
    glCreateFramebuffers(1, &m_id);
    resize(m_specification.width, m_specification.height);
}

FrameBuffer::~FrameBuffer() {
    glDeleteFramebuffers(1, &m_id);
    if (!m_textures.empty()) {
        glDeleteTextures(m_textures.size(), m_textures.data());
    }
}

void FrameBuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void FrameBuffer::bindAttachments() {
    for (std::size_t i = 0; i < m_textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
    }
}

void FrameBuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::resize(uint32_t width, uint32_t height) {
    if (!m_textures.empty()) {
        glDeleteTextures(m_textures.size(), m_textures.data());
        m_textures.clear();
        m_colorAttachments.clear();
    }

    m_specification.width = width;
    m_specification.height = height;

    m_colorAttachments.reserve(m_specification.colorAttachments.size());
    m_textures.resize(m_specification.colorAttachments.size() + 1 /* depth buffer */);
    glCreateTextures(GL_TEXTURE_2D, m_textures.size(), m_textures.data());

    for (std::size_t i = 0; i < m_specification.colorAttachments.size(); i++) {
        attachColorTexture(m_textures[i], m_specification.colorAttachments[i].type, i);
    }

    attachDepthTexture(m_textures.back(), m_specification.depthAttachment.type);

    glNamedFramebufferDrawBuffers(m_id, m_colorAttachments.size(), m_colorAttachments.data());

    assert(glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void FrameBuffer::attachColorTexture(uint32_t textureId, PixelType type, uint32_t slot) {
    glTextureStorage2D(textureId, 1, pixelFormat(type), m_specification.width, m_specification.height);

    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    m_colorAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + slot);
    glNamedFramebufferTexture(m_id, m_colorAttachments.back(), textureId, 0);
}
void FrameBuffer::attachDepthTexture(uint32_t textureId, PixelType type) {
    glTextureStorage2D(textureId, 1, pixelFormat(type), m_specification.width, m_specification.height);

    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, textureId, 0);
}
