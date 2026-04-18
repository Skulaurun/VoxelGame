#include <Engine/Core/Window.hpp>
#include <Engine/Core/Log.hpp>

#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/ChunkRenderer.hpp>
#include <Engine/Renderer/OpenGL/OpenGL.hpp>
#include <Engine/Renderer/OpenGL/Shader.hpp>
#include <Engine/Renderer/OpenGL/Texture.hpp>

#include <Engine/World/World.hpp>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <tracy/Tracy.hpp>

constexpr auto vertexShaderSource = R"(
    #version 450
    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec4 aColor;
    uniform mat4 uViewProjection;
    out vec4 vertexColor;
    void main()
    {
        gl_Position = uViewProjection * vec4(aPosition, 1.0);
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

glm::vec3 toCartesian(const float theta, const float phi) {
    return {
        glm::cos(theta) * glm::cos(phi),
        glm::sin(theta),
        glm::cos(theta) * glm::sin(phi)
    };
}

glm::vec3 cameraPosition = glm::vec3(0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
// cameraDir

float yaw = -90.0f;
float pitch = 0.0f;

float cameraSpeed = 10.0f;

void processInput(GLFWwindow* window, const float dt) {
    float speed = cameraSpeed * dt;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPosition += speed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPosition -= speed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPosition += speed * glm::normalize(glm::cross(cameraUp, cameraFront));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPosition += speed * glm::normalize(glm::cross(cameraFront, cameraUp));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cameraPosition += speed * cameraUp;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraPosition -= speed * cameraUp;
    }
}

void processMouseInput(GLFWwindow* window, double x, double y) {
    static float lastX = x;
    static float lastY = y;

    float xOffset = x - lastX;
    float yOffset = lastY - y;
    lastX = x;
    lastY = y;

    constexpr float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw = glm::mod(yaw + xOffset, 360.0f);
    pitch += yOffset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    cameraFront = glm::normalize(toCartesian(glm::radians(pitch), glm::radians(yaw)));
}

void processScroll(GLFWwindow* window, double xoffset, double yoffset) {
    cameraSpeed += yoffset;
}

int main() {
    Log::init();
    Window window(WindowProperties{ "VoxelGame", 1920, 1080 });
    Renderer::init();

    World world{};
    world.generate();
    world.setViewRange(8);

    ChunkRenderer chunkRenderer{};
    chunkRenderer.resize(8);

    Shader shader({
        { ShaderType::Vertex, vertexShaderSource },
        { ShaderType::Fragment, fragmentShaderSource },
    });
    shader.bind();

    glfwSetCursorPosCallback(window.getNativeWindow(), processMouseInput);
    glfwSetInputMode(window.getNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window.getNativeWindow(), processScroll);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window.getNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 450");

    float lastFrame = 0.0f;
    while (!window.shouldClose()) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(132, 50));
        ImGui::Begin("Overlay");
        ImGui::TextUnformatted(std::format("{:.1f} {:.1f} {:.1f}", cameraPosition.x, cameraPosition.y, cameraPosition.z).c_str());
        ImGui::End();

        Renderer::clear({ 0.1f, 0.1f, 0.1f, 1.0f });
        //Renderer::draw(*mesh.vertexArray, mesh.indices.size());
        world.render(chunkRenderer, cameraPosition);
        chunkRenderer.render();

        int width, height;
        glfwGetFramebufferSize(window.getNativeWindow(), &width, &height);

        processInput(window.getNativeWindow(), deltaTime);
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10000.0f);
        glm::mat4 viewProjection = projection * view;

        shader.setUniform("uViewProjection", viewProjection);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.display();
        FrameMark;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
