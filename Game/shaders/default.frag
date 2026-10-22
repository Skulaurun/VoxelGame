#version 450

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gAlbedo;

in vec4 vertexColor;
in vec3 normal;
in float ao;

void main()
{
    vec4 color = vertexColor;
    color.xyz *= min(ao + 0.25, 1.0);

    gAlbedo = color;
    gNormal = vec4(normalize(normal), 1.0);
}
