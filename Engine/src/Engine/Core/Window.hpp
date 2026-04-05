#pragma once

#include <cstdint>
#include <string_view>
#include <string>

struct GLFWwindow;

struct WindowProperties {

    std::string_view title;
    uint32_t width;
    uint32_t height;

    WindowProperties(
        std::string_view title = "VoxelEngine",
        uint32_t width = 1280,
        uint32_t height = 720
    ) :
        title(title), width(width), height(height)
    {}

};

class Window {

public:
    Window(const WindowProperties& properties);
    ~Window();

    void show();
    void hide();
    void focus();
    void maximize();
    void minimize();
    void restore();
    void display();
    void destroy();

    bool shouldClose() const;

    void setTitle(std::string_view title);
    std::string_view getTitle() const { return title; }

    GLFWwindow* getNativeWindow() { return window; }

private:
    GLFWwindow* window;
    static uint32_t windowCount;

    std::string title;

};
