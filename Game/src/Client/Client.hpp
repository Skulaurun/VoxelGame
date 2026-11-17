#pragma once

#include "Client/ChunkMesher.hpp"
#include "Client/ChunkRenderer.hpp"
#include "Client/World/World.hpp"

#include "Common/Common.hpp"
#include "Common/Network/Client.hpp"

#include <Engine/Renderer/Frustum.hpp>
#include <Engine/Renderer/PerspectiveCamera.hpp>
#include <Engine/Renderer/OpenGL/Shader.hpp>

#include <Engine/Core/Window.hpp>

#include <entt/entt.hpp>

#include <memory>
#include <thread>

struct GLFWwindow;

class FrameBuffer;
class VertexArray;

namespace Server {
class Server;
}

namespace Client {

class ClientHandler;

class Client {

public:
    struct Options {
        int32_t renderDistance;
        glm::vec3 sunDirection;
    };

public:
    Client();
    ~Client();

    Client(Client&&) = delete;
    Client(const Client&) = delete;

    Client& operator=(Client&&) = delete;
    Client& operator=(const Client&) = delete;

    void run();
    void update(float dt);
    void render(float dt);
    void beginFrame();
    void endFrame();

    void renderScene(
        const glm::vec3& origin,
        const glm::mat4& viewProjection,
        const Frustum& frustum,
        Shader& terrainShader,
        Shader& entityShader
    );
    void doRenderPipeline();

    void finish();

    auto isRunning() const { return !m_window.shouldClose(); }

    void onCursorPosition(GLFWwindow* window, double x, double y);
    void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset);
    void onMouseButton(GLFWwindow* window, int button, int action, int mods);
    void onKey(GLFWwindow* window, int key, int scancode, int action, int mods);
    void onFrameBufferSize(GLFWwindow* window, int width, int height);

    void updateServerTime(float serverTime);

    void setViewDistance(int32_t viewDistance);

    auto world() -> auto& { return m_world; }
    auto registry() -> auto& { return m_registry; }
    auto options() -> auto& { return m_options; }
    auto camera() -> auto& { return m_camera; }
    auto biEntityToNetworkId() { return std::tie(m_entityToNetworkId, m_networkIdToEntity); }

    // TODO: ??
    auto chunkMesher() -> auto& { return m_chunkMesher; }
    auto chunkRenderer() -> auto& { return m_chunkRenderer; }

private:
    void processInput(GLFWwindow* window);

private:
    Window m_window;
    std::unique_ptr<FrameBuffer> m_frameBuffer;
    std::unique_ptr<FrameBuffer> m_shadowFrameBuffer;
    std::unique_ptr<VertexArray> m_screenTriangle;

    Options m_options;

    Frustum m_frustum;
    Frustum m_lightFrustum;
    PerspectiveCamera m_camera;
    ChunkRenderer m_chunkRenderer;

    World m_world;
    ChunkMesher m_chunkMesher;

    float m_realTime;
    float m_simulationTime;
    float m_serverTimeOffset;
    entt::entity m_playerEntity;
    entt::registry m_registry;

    std::pair<std::shared_ptr<VertexArray>, uint32_t> m_playerMesh;

    std::unique_ptr<Shader> m_defaultShader;
    std::unique_ptr<Shader> m_lightingShader;
    std::unique_ptr<Shader> m_shadowShader;
    std::unique_ptr<Shader> m_entityShader;
    std::unique_ptr<Shader> m_debugShader;

    // TODO: Make camera controller
    float m_cameraSpeed;
    float m_yaw;
    float m_pitch;

    // TODO: move to GUI
    float m_fps;

    std::shared_ptr<ClientHandler> m_messageHandler;
    Network::Client<ClientHandler> m_netClient;

    std::unordered_map<entt::entity, NetworkId> m_entityToNetworkId;
    std::unordered_map<NetworkId, entt::entity> m_networkIdToEntity;

    // The thread needs to be destroyed before the server
    std::unique_ptr<Server::Server> m_internalServer;
    std::jthread m_serverThread;
};

}
