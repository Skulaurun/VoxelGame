#include <Engine/Core/Window.hpp>
#include <Engine/Core/Log.hpp>

#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/ChunkRenderer.hpp>
#include <Engine/Renderer/OpenGL/OpenGL.hpp>
#include <Engine/Renderer/OpenGL/Shader.hpp>
#include <Engine/Renderer/OpenGL/Texture.hpp>

#include <Engine/World/World.hpp>
#include <Engine/World/WorldGenerator.hpp>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <tracy/Tracy.hpp>

struct ShaderFile {

constexpr static char defaultVertex[] =
{
#embed "../shaders/default.vert"
};

constexpr static char defaultFragment[] =
{
#embed "../shaders/default.frag"
};

};

template<std::size_t Size>
constexpr auto embedView(const char (&data)[Size]) -> std::string_view {
    return { data, Size };
}

glm::vec3 toCartesian(const float theta, const float phi) {
    return {
        glm::cos(theta) * glm::cos(phi),
        glm::sin(theta),
        glm::cos(theta) * glm::sin(phi)
    };
}

struct GameState {
    World world;
};

glm::vec3 cameraPosition = glm::vec3(0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
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
        cameraPosition += speed * glm::normalize(glm::cross(worldUp, cameraFront));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPosition += speed * glm::normalize(glm::cross(cameraFront, worldUp));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cameraPosition += speed * worldUp;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cameraPosition -= speed * worldUp;
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

void processMouseButton(GLFWwindow* window, int button, int action, int mods) {
    auto& [world] = *static_cast<GameState*>(glfwGetWindowUserPointer(window));
    
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glm::ivec3 blockPosition = world.castRay(cameraPosition, cameraFront, 10.0f);
        Voxel* voxel = world.getVoxelAt(blockPosition);
        if (voxel) {
            voxel->setType(BlockType::Void);

            glm::ivec3 localPosition = World::toLocalPosition(blockPosition);
            glm::ivec3 chunkPosition = World::toChunkPosition(blockPosition);

            Chunk* chunk = world.getChunkAt(chunkPosition);
            if (chunk) {
                chunk->setDirty(true);
            }

            int dir[] = {-1, 1};
            int boundary[] = {0, Chunk::SIZE - 1};

            for (std::size_t i = 0; i < 3; i++) {
                for (std::size_t j = 0; j < 2; j++) {
                    if (localPosition[i] == boundary[j]) {
                        glm::ivec3 neighborPosition = chunkPosition;
                        neighborPosition[i] += dir[j];

                        Chunk* chunk = world.getChunkAt(neighborPosition);
                        if (chunk) {
                            chunk->setDirty(true);
                        }
                    }
                }
            }
        }
    }
}

int main() {
    Log::init();
    Window window(WindowProperties{ "VoxelGame", 1920, 1080 });
    Renderer::init();

    GameState state{};
    state.world.setViewRange(World::CHUNK_SIZE);

    glfwSetWindowUserPointer(window.getNativeWindow(), &state);

    WorldGenerator worldGenerator{state.world};

    worldGenerator.enqueue([](WorldGenerator::ChunkQueue& queue) {
        for (int32_t y = -World::CHUNK_SIZE; y <= World::CHUNK_SIZE; y++) {
            for (int32_t x = -World::CHUNK_SIZE; x <= World::CHUNK_SIZE; x++) {
                for (int32_t z = -World::CHUNK_SIZE; z <= World::CHUNK_SIZE; z++) {
                    queue.emplace_back(x, y, z);
                }
            }
        }
    });

    ChunkRenderer chunkRenderer{};
    chunkRenderer.resize(World::CHUNK_SIZE);

    Shader shader({
        { ShaderType::Vertex, embedView(ShaderFile::defaultVertex) },
        { ShaderType::Fragment, embedView(ShaderFile::defaultFragment) },
    });

    glfwSetCursorPosCallback(window.getNativeWindow(), processMouseInput);
    glfwSetInputMode(window.getNativeWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window.getNativeWindow(), processScroll);
    glfwSetMouseButtonCallback(window.getNativeWindow(), processMouseButton);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window.getNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 450");

    constexpr float FPS_UPDATE_PERIOD{0.25f};

    float fps = 0.0f;
    float nextFpsUpdate = FPS_UPDATE_PERIOD;

    float lastFrame = 0.0f;
    while (!window.shouldClose()) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (nextFpsUpdate <= 0.0f) {
            fps = 1.0f / deltaTime;
            nextFpsUpdate = FPS_UPDATE_PERIOD;
        } else {
            nextFpsUpdate -= deltaTime;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // [= User input =]

        int width, height;
        glfwGetFramebufferSize(window.getNativeWindow(), &width, &height);

        processInput(window.getNativeWindow(), deltaTime);
        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraFront, worldUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 10000.0f);
        glm::mat4 viewProjection = projection * view;

        // [= Render Scene =]

        Renderer::clear({ 0.1f, 0.1f, 0.1f, 1.0f });

        shader.bind();
        shader.setUniform("uViewProjection", viewProjection);

        state.world.render(chunkRenderer, cameraPosition);
        chunkRenderer.render();
        
        // [= Render UI =]

        ImGui::SetNextWindowSize(ImVec2(132, 100));
        ImGui::Begin("Overlay");
        // 1s / fps = deltaTime
        // fps = 1 / deltaTime
        ImGui::TextUnformatted(std::format("{:.1f} {:.1f} {:.1f}", cameraPosition.x, cameraPosition.y, cameraPosition.z).c_str());
        ImGui::TextUnformatted(std::format("FPS: {}", fps).c_str());
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.display();
        FrameMark;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
