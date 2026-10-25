#pragma once

#include <cstdint>
#include <vector>
#include <variant>

#include <glm/glm.hpp>

enum class PixelType : uint8_t {
	Color8,
	Float16,
	Depth24
};

enum class TextureFilter {
    Nearest, Linear
};

struct TextureWrap {
    struct Repeat{};
    struct ClampToEdge{};
    struct ClampToBorder{
        glm::vec4 borderColor;
    };

    std::variant<Repeat, ClampToEdge, ClampToBorder> data;
};

class FrameBuffer {

public:
    struct Attachment {
        PixelType type{PixelType::Color8};
        TextureFilter filter{TextureFilter::Linear};
        TextureWrap wrap{TextureWrap::Repeat{}};
    };

    struct Specification {
        uint32_t width;
        uint32_t height;

        std::vector<Attachment> colorAttachments;
        Attachment depthAttachment;
    };

public:
	FrameBuffer(const Specification& specification);
	~FrameBuffer();

    void bind();
    void unbind();

    void bindAttachments(uint32_t offset = 0);

    void resize(uint32_t width, uint32_t height);

    auto specification() const -> const auto& { return m_specification; }
    auto attachmentCount() const { return m_textures.size(); }

	uint32_t id() const { return m_id; }

private:
    void setParameters(uint32_t textureId, const Attachment& attachment);

    void attachColorTexture(uint32_t textureId, const Attachment& attachment, uint32_t slot);
    void attachDepthTexture(uint32_t textureId, const Attachment& attachment);

private:
	uint32_t m_id;
    Specification m_specification;

    std::vector<uint32_t> m_textures;
    std::vector<uint32_t> m_colorAttachments;

};
