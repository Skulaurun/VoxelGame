#include "Client.hpp"
#include "NetCode.hpp"
#include "ShaderFile.hpp"

#include "Server/Server.hpp"

#include <Engine/Renderer/OpenGL/FrameBuffer.hpp>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <tracy/Tracy.hpp>

#include <format>

namespace Client {

constexpr glm::ivec2 DEFAULT_WINDOW_SIZE{ 1920, 1080 };

constexpr float FOV_Y = 45.0f;

constexpr int32_t CHUNK_RENDER_DISTANCE{ 8 };
constexpr int32_t RENDER_DISTANCE{ (CHUNK_RENDER_DISTANCE + 1) * Chunk::SIZE };

auto calculateLightSpaceTransform(
    const PerspectiveCamera& camera,
    const glm::vec3& lightDirection,
    const glm::vec2& shadowResolution
) {
    const auto midpoint = (camera.near() + camera.far()) / 2.0f;
    const auto cameraCenter = camera.position() + midpoint * camera.direction();

    const auto lightView = glm::lookAt(
        cameraCenter + lightDirection,
        cameraCenter,
        WORLD_UP
    );

    const auto lightCenter = glm::vec3{lightView * glm::vec4{cameraCenter, 1.0f}};

    float radius{0.0f};
    float minZ{std::numeric_limits<float>::max()};
    float maxZ{std::numeric_limits<float>::lowest()};
    for (const auto& worldPoint : Frustum::worldBounds(camera.viewProjection())) {
        const auto point = glm::vec3{lightView * glm::vec4{worldPoint, 1.0f}};
        minZ = glm::min(minZ, point.z);
        maxZ = glm::max(maxZ, point.z);
        radius = glm::max(radius, glm::distance(lightCenter, point));
    }

    // Fix floating point precision
    radius = glm::ceil(radius);

    // Extend light frustum in Z
    constexpr auto zMult{2.0f};
    if (minZ < 0) {
        minZ *= zMult;
    } else {
        minZ /= zMult;
    }
    if (maxZ < 0) {
        maxZ /= zMult;
    } else {
        maxZ *= zMult;
    }
    //maxZ += 100.0f;

    // Directional sun
    const auto lightProjection = glm::ortho(
        -radius, radius,
        -radius, radius,
        -maxZ, -minZ
    );
    
    auto lightViewProjection = lightProjection * lightView;

    // Snap to texel grid
    const auto originClip = lightViewProjection * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
    const auto originNdc = glm::vec3{originClip} / originClip.w;
    const auto originTexel = glm::vec2{originNdc} * (shadowResolution / 2.0f);
    auto offset = glm::round(originTexel) - originTexel;
    offset *= 2.0f / shadowResolution;
    lightViewProjection[3][0] += offset.x;
    lightViewProjection[3][1] += offset.y;

    return lightViewProjection;
}

auto isControllingCamera(GLFWwindow* window) {
    return glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED;
}

Client::Client() :
    m_window{WindowProperties{ "VoxelGame", DEFAULT_WINDOW_SIZE.x, DEFAULT_WINDOW_SIZE.y }},
    m_camera{FOV_Y, DEFAULT_WINDOW_SIZE.x / static_cast<float>(DEFAULT_WINDOW_SIZE.y), 0.1f, RENDER_DISTANCE},

    m_options{
        .sunDirection = { 0.4f, 0.8f, 0.3f }
    },

    m_world{},
    m_chunkMesher{m_world},

    m_cameraSpeed{10.0f},
    m_yaw{-90.0f},
    m_pitch{0.0f},

    m_fps{0.0f},

    m_messageHandler{std::make_shared<ClientHandler>(*this)},
    m_netClient{m_messageHandler}
{
    // GLFW init

    GLFWwindow* window = m_window.nativeWindow();

    glfwSetWindowUserPointer(window, this);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
        Client& client = *static_cast<Client*>(glfwGetWindowUserPointer(window));
        client.onCursorPosition(window, x, y);
    });
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        Client& client = *static_cast<Client*>(glfwGetWindowUserPointer(window));
        client.onMouseScroll(window, xoffset, yoffset);
    });
    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        Client& client = *static_cast<Client*>(glfwGetWindowUserPointer(window));
        client.onMouseButton(window, button, action, mods);
    });
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        Client& client = *static_cast<Client*>(glfwGetWindowUserPointer(window));
        client.onKey(window, key, scancode, action, mods);
    });
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        Client& client = *static_cast<Client*>(glfwGetWindowUserPointer(window));
        client.onFrameBufferSize(window, width, height);
    });

    // Renderer init

    Renderer::init();

    m_chunkRenderer.init();

    m_frameBuffer = std::make_unique<FrameBuffer>(FrameBuffer::Specification{
        static_cast<uint32_t>(DEFAULT_WINDOW_SIZE.x),
        static_cast<uint32_t>(DEFAULT_WINDOW_SIZE.y),
        {
            { PixelType::Float16 },
            { PixelType::Color8 }
        },
        { PixelType::Depth24, TextureFilter::Nearest }
    });

    m_shadowFrameBuffer = std::make_unique<FrameBuffer>(FrameBuffer::Specification{
        // TODO: implement Cascaded Shadow Mapping
        8192, 8192,
        {},
        { PixelType::Depth24, TextureFilter::Nearest, TextureWrap::ClampToBorder{glm::vec4{0.0}} }
    });

    m_screenTriangle = std::make_unique<VertexArray>();

    m_defaultShader = std::make_unique<Shader>(std::vector<ShaderSource>{
        { ShaderType::Vertex, embedView(ShaderFile::defaultVertex) },
        { ShaderType::Fragment, embedView(ShaderFile::defaultFragment) },
    });

    m_lightingShader = std::make_unique<Shader>(std::vector<ShaderSource>{
        { ShaderType::Vertex, embedView(ShaderFile::lightingVertex) },
        { ShaderType::Fragment, embedView(ShaderFile::lightingFragment) },
    });

    m_shadowShader = std::make_unique<Shader>(std::vector<ShaderSource>{
        { ShaderType::Vertex, embedView(ShaderFile::shadowVertex) }
    });

    m_debugShader = std::make_unique<Shader>(std::vector<ShaderSource>{
        { ShaderType::Vertex, embedView(ShaderFile::lightingVertex) },
        { ShaderType::Fragment, embedView(ShaderFile::debugFragment) }
    });

    m_lightingShader->bind();
    m_lightingShader->setUniform("uNormal", 0);
    m_lightingShader->setUniform("uAlbedo", 1);
    m_lightingShader->setUniform("uDepth", 2);
    m_lightingShader->setUniform("uShadowMap", 3);

    // ImGui init

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    //ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    // Network init

    m_netClient.start();

    if (true) {
        // Singleplayer only
        m_internalServer = std::make_unique<Server::Server>();
        m_serverThread = std::jthread{[this] {
            m_internalServer->run();
        }};
        m_internalServer->netServer().connect(m_netClient.createConnection());
    } else {
        m_netClient.connect("127.0.0.1", 5500);
    }
}

Client::~Client() {
    m_netClient.disconnect();
    m_netClient.stop();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_internalServer) {
        m_internalServer->stop();
    }
}

void Client::onFrameBufferSize(GLFWwindow* window, int width, int height) {
    if (width == 0 || height == 0) {
        return;
    }

    //Renderer::setViewport({ width, height });
    m_camera.setAspectRatio(width / static_cast<float>(height));
    m_frameBuffer->resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}

void Client::onMouseButton(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // glm::ivec3 blockPosition = m_world.castRay(m_camera.position(), m_camera.direction(), 10.0f);
        // Voxel* voxel = m_world.getVoxelAt(blockPosition);
        // if (voxel) {
        //     *voxel = BlockType::Void;

        //     glm::ivec3 localPosition = toLocalPosition(blockPosition);
        //     glm::ivec3 chunkPosition = toChunkPosition(blockPosition);

        //     Chunk* chunk = m_world.getChunkAt(chunkPosition);
        //     // if (chunk) {
        //     //     chunk->dirty = true;
        //     // }

        //     int dir[] = {-1, 1};
        //     int boundary[] = {0, Chunk::SIZE - 1};

        //     for (std::size_t i = 0; i < 3; i++) {
        //         for (std::size_t j = 0; j < 2; j++) {
        //             if (localPosition[i] == boundary[j]) {
        //                 glm::ivec3 neighborPosition = chunkPosition;
        //                 neighborPosition[i] += dir[j];

        //                 Chunk* chunk = m_world.getChunkAt(neighborPosition);
        //                 // if (chunk) {
        //                 //     chunk->dirty = true;
        //                 // }
        //             }
        //         }
        //     }
        // }
    }
}

void Client::onKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR,
            isControllingCamera(window) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED
        );
    }
}

void Client::onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
    m_cameraSpeed += yoffset;
}

void Client::onCursorPosition(GLFWwindow* window, double x, double y) {
    static float lastX = x;
    static float lastY = y;

    float xOffset = x - lastX;
    float yOffset = lastY - y;
    lastX = x;
    lastY = y;

    if (!isControllingCamera(window)) {
        return;
    }

    constexpr float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    m_yaw = glm::mod(m_yaw + xOffset, 360.0f);
    m_pitch += yOffset;

    if (m_pitch > 89.0f) {
        m_pitch = 89.0f;
    }
    if (m_pitch < -89.0f) {
        m_pitch = -89.0f;
    }

    m_camera.setDirection(
        glm::normalize(toCartesian(glm::radians(m_pitch), glm::radians(m_yaw)))
    );
}

void Client::processInput(GLFWwindow* window, const float dt) {
    float speed = m_cameraSpeed * dt;

    WorldPosition position = m_camera.position();
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += speed * m_camera.direction();
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= speed * m_camera.direction();
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position += speed * glm::normalize(glm::cross(WORLD_UP, m_camera.direction()));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += speed * glm::normalize(glm::cross(m_camera.direction(), WORLD_UP));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        position += speed * WORLD_UP;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        position -= speed * WORLD_UP;
    }
    m_camera.setPosition(position);
}

void Client::renderScene(const Frustum& frustum) {
    m_chunkRenderer.render(frustum);
}

void Client::doRenderPipeline() {
    m_frustum.update(m_camera.viewProjection());

    const auto lightViewProjection = calculateLightSpaceTransform(m_camera, m_options.sunDirection, {
        m_shadowFrameBuffer->specification().width,
        m_shadowFrameBuffer->specification().height
    });

    m_lightFrustum.update(lightViewProjection);

    { // Shadow pass
        const auto& spec = m_shadowFrameBuffer->specification();
        Renderer::setViewport({ spec.width, spec.height });
        m_shadowFrameBuffer->bind();
        Renderer::clear(glm::vec4{});

        m_shadowShader->bind();
        m_shadowShader->setUniform("uViewProjection", lightViewProjection);

        renderScene(m_lightFrustum);

        m_shadowFrameBuffer->unbind();

        // DEBUG
        // NOTE: missing setViewport (moved below)
        // Renderer::clear({ 0.529f, 0.808f, 0.922f, 1.0f });
        // m_debugShader->bind();
        // m_shadowFrameBuffer->bindAttachments();
        // m_screenTriangle->bind();
        // Renderer::draw(3);
    }

    { // Geometry pass
        const auto& spec = m_frameBuffer->specification();
        Renderer::setViewport({ spec.width, spec.height });
        m_frameBuffer->bind();
        Renderer::clear(glm::vec4{});

        m_defaultShader->bind();
        m_defaultShader->setUniform("uViewProjection", m_camera.viewProjection());

        renderScene(m_frustum);

        m_frameBuffer->unbind();
    }

    { // Lighting pass
        // FIXME: does nothing
        Renderer::clear({ 0.529f, 0.808f, 0.922f, 1.0f });
    
        m_lightingShader->bind();
        m_lightingShader->setUniform("uSunDirection", m_options.sunDirection);
        m_lightingShader->setUniform("uSunColor", glm::vec3{1.0f, 0.95f, 0.85f});
        m_lightingShader->setUniform("uSunIntensity", 1.0f);
        m_lightingShader->setUniform("uAmbientColor", glm::vec3{0.55f, 0.60f, 0.70f});
        m_lightingShader->setUniform("uSkyColor", glm::vec3{0.60f, 0.85f, 1.0f});
        m_lightingShader->setUniform("uViewProjection", m_camera.viewProjection());
        m_lightingShader->setUniform("uLightViewProjection", lightViewProjection);

        m_frameBuffer->bindAttachments(0);
        m_shadowFrameBuffer->bindAttachments(m_frameBuffer->attachmentCount());

        m_screenTriangle->bind();
        Renderer::draw(3);
    }
}

void Client::update(const float dt) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // [= Server events =]

    m_netClient.update();

    m_chunkMesher.drain([this](ChunkMesher::Result& result) {
        if (auto* chunk = m_world.chunkAt(result.position)) {
            m_chunkRenderer.loadMesh(result.position, result.mesh);
            chunk->mesh = std::move(result.mesh);
        }
    });

    // [= User input =]

    processInput(m_window.nativeWindow(), dt);

    { // TODO: temporary solution
        Common::Network::Message message{ .id = Common::Network::MessageId::PlayerMove };
        Common::Network::ProtocolStream& stream = message.stream;
        stream.write(m_camera.position());
        m_netClient.send(message);
    }

    // [= Render Scene =]
    doRenderPipeline();
    
    // [= Render UI =]

    ImGui::SetNextWindowSize(ImVec2(132, 100));
    ImGui::Begin("Overlay");
    ImGui::TextUnformatted(
        std::format("{:.1f} {:.1f} {:.1f}", m_camera.position().x, m_camera.position().y, m_camera.position().z).c_str()
    );
    // 1s / fps = deltaTime
    // fps = 1 / deltaTime
    ImGui::TextUnformatted(
        std::format("FPS: {}", m_fps).c_str()
    );
    ImGui::TextUnformatted(
        std::format("Draw calls: {}", m_chunkRenderer.statistics().numDrawCalls).c_str()
    );
    ImGui::End();

    if (!isControllingCamera(m_window.nativeWindow())) {
        ImGui::Begin("Options");
        ImGui::SliderFloat3("Sun Direction", glm::value_ptr(m_options.sunDirection), -1.0f, 1.0f);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    m_window.display();
    FrameMark;
}

void Client::run() {
    constexpr float FPS_UPDATE_PERIOD{0.25f};

    float nextFpsUpdate = FPS_UPDATE_PERIOD;

    float lastFrame = 0.0f;
    while (isRunning()) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (nextFpsUpdate <= 0.0f) {
            m_fps = 1.0f / deltaTime;
            nextFpsUpdate = FPS_UPDATE_PERIOD;
        } else {
            nextFpsUpdate -= deltaTime;
        }

        update(deltaTime);        
    }

    finish();
}

void Client::finish() {}

}
