#include "Window.hpp"

#include <GLFW/glfw3.h>

#include <cassert>

uint32_t Window::windowCount = 0;

Window::Window(const WindowProperties& properties) {
    if (windowCount == 0) {
        int result = glfwInit();
        assert(result && "Could not initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    window = glfwCreateWindow(
        properties.width,
        properties.height,
        properties.title.data(),
        NULL, NULL
    );
    assert(window && "Could not create GLFW Window");

    glfwMakeContextCurrent(window);
    windowCount++;
}

Window::~Window() {
    destroy();
}

void Window::show() {
    glfwShowWindow(window);
}

void Window::hide() {
    glfwHideWindow(window);
}

void Window::focus() {
    glfwFocusWindow(window);
}

void Window::maximize() {
    glfwMaximizeWindow(window);
}

void Window::minimize() {
    glfwIconifyWindow(window);
}

void Window::restore() {
    glfwRestoreWindow(window);
}

void Window::display() {
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void Window::destroy() {
    glfwDestroyWindow(window);
    if (--windowCount == 0) {
        glfwTerminate();
    }
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::setTitle(std::string_view title) {
    glfwSetWindowTitle(window, title.data());
    this->title = title;
}
