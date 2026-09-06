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
    int version = gladLoadGL(glfwGetProcAddress);
    assert(version != 0);

    glDebugMessageCallback(messageCallback, nullptr);

    glDebugMessageControl(
        GL_DONT_CARE,
        GL_DONT_CARE,
        GL_DEBUG_SEVERITY_NOTIFICATION,
        0, nullptr,
        GL_FALSE
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}
