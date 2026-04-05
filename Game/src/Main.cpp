#include <Engine/Core/Window.hpp>
#include <Engine/Core/Log.hpp>

#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/OpenGL/OpenGL.hpp>
#include <Engine/Renderer/OpenGL/Shader.hpp>
#include <Engine/Renderer/OpenGL/Texture.hpp>
#include <Engine/Renderer/OpenGL/IndexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexBuffer.hpp>
#include <Engine/Renderer/OpenGL/VertexArray.hpp>

constexpr auto vertexShaderSource = R"(
    #version 450
    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec4 aColor;
    out vec4 vertexColor;
    void main()
    {
        gl_Position = vec4(aPosition, 1.0);
        vertexColor = aColor;
    }
)";

constexpr auto fragmentShaderSource = R"(
    #version 450
    out vec4 fragment;
    in vec4 vertexColor;
    void main()
    {
        fragment = vertexColor;
    }
)";

int main() {
    Log::init();
    Window window(WindowProperties{ "VoxelGame", 1920, 1080 });
    Renderer::init();

    std::vector<uint32_t> indices{
        0, 1, 2,
        1, 3, 2,
    };

    std::vector<Vertex> vertices{
        Vertex{ glm::vec3{ -0.5f, -0.5f, 0.0f }, glm::vec4{ 1.0f, 1.0f, 0.0f, 1.0f }, glm::vec2{}, 0.0f },
        Vertex{ glm::vec3{ 0.5f, -0.5f, 0.0f }, glm::vec4{ 0.0f, 1.0f, 1.0f, 1.0f }, glm::vec2{}, 0.0f },
        Vertex{ glm::vec3{ -0.5f, 0.5f, 0.0f }, glm::vec4{ 1.0f, 0.0f, 1.0f, 1.0f }, glm::vec2{}, 0.0f },
        Vertex{ glm::vec3{ 0.5f, 0.5f, 0.0f }, glm::vec4{ 1.0f, 0.0f, 1.0f, 1.0f }, glm::vec2{}, 0.0f }
    };

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices.data(), indices.size() * sizeof(uint32_t));
    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(Vertex));
    vertexBuffer->setLayout({
        VertexAttributeType::Float3,
        VertexAttributeType::Float4,
        VertexAttributeType::Float2,
        VertexAttributeType::Float
    });

    std::shared_ptr<VertexArray> vertexArray = std::make_shared<VertexArray>();
    vertexArray->addVertexBuffer(vertexBuffer);
    vertexArray->setIndexBuffer(indexBuffer);

    Shader shader({
        { ShaderType::Vertex, vertexShaderSource },
        { ShaderType::Fragment, fragmentShaderSource },
    });
    shader.bind();

    while (!window.shouldClose()) {
        Renderer::clear({ 0.1f, 0.1f, 0.1f, 1.0f });
        Renderer::draw(*vertexArray, indices.size());

        window.display();
    }
}
