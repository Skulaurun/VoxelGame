#pragma once

#include <tracy/Tracy.hpp>

#include <glm/glm.hpp>

#include <array>

inline float blend(float t) {
    return 6 * glm::pow(t, 5) - 15 * glm::pow(t, 4) + 10 * glm::pow(t, 3);
}

inline float lerp(float a, float b, float t) {
    return a * (1 - t) + b * t;
}

constexpr int16_t permutationTable[] = {
    201, 179, 55, 112, 20, 80, 226, 71, 227, 158, 151, 249, 200, 174, 239, 213, 34, 15, 14, 88, 51, 234, 129, 104, 7, 140, 177, 95, 70, 126, 52, 91,
    127, 150, 121, 230, 41, 182, 27, 164, 181, 250, 77, 202, 243, 147, 59, 35, 1, 216, 97, 63, 3, 237, 62, 125, 137, 223, 46, 228, 82, 152, 205, 193,
    244, 246, 236, 117, 98, 68, 79, 87, 232, 196, 78, 192, 225, 32, 159, 235, 194, 100, 124, 94, 207, 115, 195, 215, 44, 251, 133, 120, 212, 21, 29, 139,
    255, 218, 253, 220, 17, 191, 58, 153, 10, 61, 48, 43, 128, 241, 18, 93, 242, 135, 143, 40, 111, 245, 208, 204, 5, 114, 83, 103, 206, 119, 132, 0,
    184, 13, 37, 155, 154, 178, 113, 240, 67, 86, 54, 222, 130, 142, 169, 9, 31, 176, 229, 39, 123, 89, 161, 101, 109, 168, 57, 19, 110, 233, 65, 221,
    157, 197, 156, 187, 2, 122, 144, 75, 248, 238, 231, 210, 30, 211, 99, 24, 186, 165, 254, 141, 69, 224, 47, 170, 209, 149, 33, 175, 72, 38, 219, 180,
    252, 171, 190, 173, 23, 12, 131, 6, 53, 50, 102, 203, 16, 172, 160, 162, 73, 166, 28, 25, 116, 189, 105, 45, 247, 96, 108, 8, 26, 84, 134, 4,
    163, 92, 90, 64, 183, 198, 66, 188, 22, 167, 185, 145, 60, 56, 138, 85, 118, 136, 42, 199, 11, 214, 148, 107, 106, 217, 81, 146, 49, 74, 76, 36,

    201, 179, 55, 112, 20, 80, 226, 71, 227, 158, 151, 249, 200, 174, 239, 213, 34, 15, 14, 88, 51, 234, 129, 104, 7, 140, 177, 95, 70, 126, 52, 91,
    127, 150, 121, 230, 41, 182, 27, 164, 181, 250, 77, 202, 243, 147, 59, 35, 1, 216, 97, 63, 3, 237, 62, 125, 137, 223, 46, 228, 82, 152, 205, 193,
    244, 246, 236, 117, 98, 68, 79, 87, 232, 196, 78, 192, 225, 32, 159, 235, 194, 100, 124, 94, 207, 115, 195, 215, 44, 251, 133, 120, 212, 21, 29, 139,
    255, 218, 253, 220, 17, 191, 58, 153, 10, 61, 48, 43, 128, 241, 18, 93, 242, 135, 143, 40, 111, 245, 208, 204, 5, 114, 83, 103, 206, 119, 132, 0,
    184, 13, 37, 155, 154, 178, 113, 240, 67, 86, 54, 222, 130, 142, 169, 9, 31, 176, 229, 39, 123, 89, 161, 101, 109, 168, 57, 19, 110, 233, 65, 221,
    157, 197, 156, 187, 2, 122, 144, 75, 248, 238, 231, 210, 30, 211, 99, 24, 186, 165, 254, 141, 69, 224, 47, 170, 209, 149, 33, 175, 72, 38, 219, 180,
    252, 171, 190, 173, 23, 12, 131, 6, 53, 50, 102, 203, 16, 172, 160, 162, 73, 166, 28, 25, 116, 189, 105, 45, 247, 96, 108, 8, 26, 84, 134, 4,
    163, 92, 90, 64, 183, 198, 66, 188, 22, 167, 185, 145, 60, 56, 138, 85, 118, 136, 42, 199, 11, 214, 148, 107, 106, 217, 81, 146, 49, 74, 76, 36
};

inline float perlinNoise(const glm::vec3& point) {
    ZoneScoped;
    glm::ivec3 ip{ glm::floor(point.x), glm::floor(point.y), glm::floor(point.z) };
    glm::vec3 fp = point - glm::vec3(ip);

    ip.x &= 255;
    ip.y &= 255;
    ip.z &= 255;

    const std::array<glm::ivec3, 8> gridCorners{{
        {ip.x, ip.y, ip.z},
        {ip.x, ip.y, ip.z + 1},
        {ip.x, ip.y + 1, ip.z + 1},
        {ip.x, ip.y + 1, ip.z},
        {ip.x + 1, ip.y, ip.z},
        {ip.x + 1, ip.y, ip.z + 1},
        {ip.x + 1, ip.y + 1, ip.z + 1},
        {ip.x + 1, ip.y + 1, ip.z}
    }};

    const std::array<glm::vec3, 8> gridCornerToPoint{{
        {fp.x, fp.y, fp.z},
        {fp.x, fp.y, fp.z - 1},
        {fp.x, fp.y - 1, fp.z - 1},
        {fp.x, fp.y - 1, fp.z},
        {fp.x - 1, fp.y, fp.z},
        {fp.x - 1, fp.y, fp.z - 1},
        {fp.x - 1, fp.y - 1, fp.z - 1},
        {fp.x - 1, fp.y - 1, fp.z}
    }};

    constexpr glm::vec3 GRADIENTS_3D[] = {
        { 0, 1, 1 }, { 0, 1, -1 },
        { 0, -1, 1 }, { 0, -1, -1 },
        { 1, 0, 1 }, { 1, 0, -1 },
        { -1, 0, 1 }, { -1, 0, -1 },
        { 1, 1, 0 }, { 1, -1, 0 },
        { -1, 1, 0 }, { -1, -1, 0 }
    };

    std::array<float, 8> weights{};
    for (std::size_t i = 0; i < weights.size(); i++) {
        auto& c = gridCorners[i];
        auto h = permutationTable[c.x + permutationTable[c.y + permutationTable[c.z]]];

        weights[i] = glm::dot(GRADIENTS_3D[h % 12], gridCornerToPoint[i]);
    }

    const glm::vec3 fade{ blend(fp.x), blend(fp.y), blend(fp.z) };

    // Trilinear interpolation
    float c00 = lerp(weights[0], weights[4], fade.x);
    float c01 = lerp(weights[1], weights[5], fade.x);
    float c10 = lerp(weights[3], weights[7], fade.x);
    float c11 = lerp(weights[2], weights[6], fade.x);

    float c0 = lerp(c00, c10, fade.y);
    float c1 = lerp(c01, c11, fade.y);

    return lerp(c0, c1, fade.z);
}

inline float perlinNoise(const glm::vec2& point) {
    ZoneScoped;
    glm::ivec2 ip{ glm::floor(point.x), glm::floor(point.y) };
    glm::vec2 fp = point - glm::vec2(ip);

    ip.x &= 255;
    ip.y &= 255;

    const std::array<glm::ivec2, 4> gridCorners{{
        {ip.x, ip.y},
        {ip.x + 1, ip.y},
        {ip.x + 1, ip.y + 1},
        {ip.x, ip.y + 1}
    }};

    const std::array<glm::vec2, 4> gridCornerToPoint{{
        {fp.x, fp.y},
        {fp.x - 1, fp.y},
        {fp.x - 1, fp.y - 1},
        {fp.x, fp.y - 1}
    }};

    constexpr glm::vec2 GRADIENTS_2D[] = {
        { 0, 1 }, { 1, 0 }, { 1, 1 },
        { 0, -1 }, { -1, 0 }, { -1, -1 },
        { -1, 1 }, { 1, -1 }
    };

    std::array<float, 4> weights{};
    for (std::size_t i = 0; i < weights.size(); i++) {
        auto& c = gridCorners[i];
        auto h = permutationTable[c.x + permutationTable[c.y]];

        weights[i] = glm::dot(GRADIENTS_2D[h % 8], gridCornerToPoint[i]);
    }

    const glm::vec2 fade{ blend(fp.x), blend(fp.y) };

    float c0 = lerp(weights[0], weights[1], fade.x);
    float c1 = lerp(weights[3], weights[2], fade.x);

    return lerp(c0, c1, fade.y);
}

inline float fractalBrownianMotion(const glm::vec3& point) {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.005f;

    constexpr auto OCTAVE_COUNT = 8;
    for (std::size_t i = 0; i < OCTAVE_COUNT; i++) {
        value += amplitude * perlinNoise(point * frequency);
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return value;
}
