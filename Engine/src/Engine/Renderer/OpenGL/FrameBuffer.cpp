#include <Engine/Renderer/OpenGL/FrameBuffer.hpp>
#include <Engine/Renderer/OpenGL/Texture.hpp>

#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>

#include <cassert>

auto toGlPixelFormat(PixelType type) -> GLenum {
    switch (type) {
        case PixelType::Color8: return GL_RGBA8;
        case PixelType::Float16: return GL_RGBA16F;
        case PixelType::Depth24: return GL_DEPTH_COMPONENT24;
        default: return GL_NONE;
    }
}

auto toGlTextureFilter(TextureFilter type) -> GLenum {
    switch (type) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear: return GL_LINEAR;
        default: return GL_NONE;
    }
}

template<typename Type> struct WrapTraits { constexpr static GLenum value = GL_NONE; };
template<> struct WrapTraits<TextureWrap::Repeat> { constexpr static GLenum value = GL_REPEAT; };
template<> struct WrapTraits<TextureWrap::ClampToEdge> { constexpr static GLenum value = GL_CLAMP_TO_EDGE; };
template<> struct WrapTraits<TextureWrap::ClampToBorder> { constexpr static GLenum value = GL_CLAMP_TO_BORDER; };

auto toGlTextureWrap(const TextureWrap& wrap) -> GLenum {
    return std::visit([]<typename Type>(Type&& type) {
        return WrapTraits<std::decay_t<Type>>::value;
    }, wrap.data);
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

void FrameBuffer::bindAttachments(uint32_t offset) {
    for (std::size_t i = 0; i < m_textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + offset + i);
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
        attachColorTexture(m_textures[i], m_specification.colorAttachments[i], i);
    }

    attachDepthTexture(m_textures.back(), m_specification.depthAttachment);

    if (!m_colorAttachments.empty()) {
        glNamedFramebufferDrawBuffers(m_id, m_colorAttachments.size(), m_colorAttachments.data());
    } else {
        glNamedFramebufferDrawBuffer(m_id, GL_NONE);
        glNamedFramebufferReadBuffer(m_id, GL_NONE);
    }

    assert(glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

void FrameBuffer::setParameters(uint32_t textureId, const Attachment& attachment) {
    glTextureStorage2D(textureId, 1, toGlPixelFormat(attachment.type), m_specification.width, m_specification.height);

    const auto filter = toGlTextureFilter(attachment.filter);
    const auto wrap = toGlTextureWrap(attachment.wrap);
    glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, filter);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, wrap);

    if (const auto* clampToBorder = std::get_if<TextureWrap::ClampToBorder>(&attachment.wrap.data)) {
        glTextureParameterfv(textureId, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(clampToBorder->borderColor));
    }
}

void FrameBuffer::attachColorTexture(uint32_t textureId, const Attachment& attachment, uint32_t slot) {
    setParameters(textureId, attachment);

    m_colorAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + slot);
    glNamedFramebufferTexture(m_id, m_colorAttachments.back(), textureId, 0);
}

void FrameBuffer::attachDepthTexture(uint32_t textureId, const Attachment& attachment) {
    setParameters(textureId, attachment);

    glNamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, textureId, 0);
}
