#pragma once

#include "Client/ChunkMesher.hpp"
#include "Client/ChunkRenderer.hpp"
#include "Client/World/World.hpp"
#include "Common/Network/Client.hpp"

#include <Engine/Renderer/Frustum.hpp>
#include <Engine/Renderer/PerspectiveCamera.hpp>
#include <Engine/Renderer/OpenGL/Shader.hpp>

#include <Engine/Core/Window.hpp>

#include <memory>
#include <thread>

struct GLFWwindow;

namespace Server {
class Server;
}

namespace Client {

class ClientHandler;

class Client {

public:
    Client();
    ~Client();

    Client(Client&&) = delete;
    Client(const Client&) = delete;

    Client& operator=(Client&&) = delete;
    Client& operator=(const Client&) = delete;

    void run();
    void update(const float dt);

    void finish();

    auto isRunning() const { return !m_window.shouldClose(); }

    void onCursorPosition(GLFWwindow* window, double x, double y);
    void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset);
    void onMouseButton(GLFWwindow* window, int button, int action, int mods);
    void onFrameBufferSize(GLFWwindow* window, int width, int height);

    auto world() -> auto& { return m_world; }

    // TODO: ??
    auto chunkMesher() -> auto& { return m_chunkMesher; }
    auto chunkRenderer() -> auto& { return m_chunkRenderer; }

private:
    void processInput(GLFWwindow* window, const float dt);

private:
    Window m_window;

    Frustum m_frustum;
    PerspectiveCamera m_camera;
    ChunkRenderer m_chunkRenderer;
    
    World m_world;
    ChunkMesher m_chunkMesher;

    std::unique_ptr<Shader> m_defaultShader;

    // TODO: Make camera controller
    float m_cameraSpeed;
    float m_yaw;
    float m_pitch;

    // TODO: move to GUI
    float m_fps;

    std::shared_ptr<ClientHandler> m_messageHandler;
    Common::Network::Client<ClientHandler> m_netClient;

    // The thread needs to be destroyed before the server
    std::unique_ptr<Server::Server> m_internalServer;
    std::jthread m_serverThread;
};

}
