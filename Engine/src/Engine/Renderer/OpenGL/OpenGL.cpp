#include <Engine/Renderer/OpenGL/OpenGL.hpp>
#include <Engine/Core/Log.hpp>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

void messageCallback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:         LOG_CORE_CRITICAL(message); return;
        case GL_DEBUG_SEVERITY_MEDIUM:       LOG_CORE_ERROR(message); return;
        case GL_DEBUG_SEVERITY_LOW:          LOG_CORE_WARN(message); return;
        case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_CORE_INFO(message); return;
    }

    assert(false && "Unknown severity");
}

void OpenGL::init() {
    gladLoadGL(glfwGetProcAddress);

    glDebugMessageCallback(messageCallback, nullptr);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
}
