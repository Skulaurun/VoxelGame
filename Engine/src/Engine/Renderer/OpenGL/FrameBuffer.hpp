#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <memory>

enum class PixelType : uint8_t {
	Color8,
	Float16,
	Depth24
};

class FrameBuffer {

public:
    struct Attachment {
        PixelType type;
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

    void bindAttachments();

    void resize(uint32_t width, uint32_t height);

	uint32_t id() const { return m_id; }

private:
    void attachColorTexture(uint32_t textureId, PixelType type, uint32_t slot);
    void attachDepthTexture(uint32_t textureId, PixelType type);

private:
	uint32_t m_id;
    Specification m_specification;

    std::vector<uint32_t> m_textures;
    std::vector<uint32_t> m_colorAttachments;

};
