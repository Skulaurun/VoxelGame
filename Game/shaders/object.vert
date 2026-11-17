#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uViewProjection;

out vec3 normal;

void main()
{
    gl_Position = uViewProjection * uModel * vec4(aPosition, 1.0);

    normal = aNormal;
}
