#pragma once

#include "Client/ChunkRenderer.hpp"

#include <Engine/Renderer/Frustum.hpp>
#include <Engine/Renderer/PerspectiveCamera.hpp>
#include <Engine/Renderer/OpenGL/Shader.hpp>

#include <Engine/Core/Window.hpp>

#include <memory>
#include <thread>

struct GLFWwindow;

class Server;

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

    auto isRunning() const { return !m_window.shouldClose(); }

    void onCursorPosition(GLFWwindow* window, double x, double y);
    void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset);
    void onMouseButton(GLFWwindow* window, int button, int action, int mods);
    void onFrameBufferSize(GLFWwindow* window, int width, int height);

private:
    void processInput(GLFWwindow* window, const float dt);

private:
    Window m_window;

    Frustum m_frustum;
    PerspectiveCamera m_camera;
    ChunkRenderer m_chunkRenderer;

    std::unique_ptr<Shader> m_defaultShader;

    // TODO: Make camera controller
    float m_cameraSpeed;
    float m_yaw;
    float m_pitch;

    // TODO: move to GUI
    float m_fps;

    // TODO: Add client Connection

    std::jthread m_serverThread;
    std::unique_ptr<Server> m_internalServer;
};
