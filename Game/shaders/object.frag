#version 450

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gAlbedo;

in vec3 normal;

void main()
{
    vec4 color = vec4(0.45, 0.70, 1.0, 1.0);

    gAlbedo = color;
    // Multiply by normalMatrix?
    gNormal = vec4(normalize(normal), 1.0);
}
