#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;
layout (location = 5) in float aAO;

uniform mat4 uModel;
uniform mat4 uViewProjection;

out vec4 vertexColor;
out vec3 normal;
out float ao;

void main()
{
    gl_Position = uViewProjection * uModel * vec4(aPosition, 1.0);
    
    vertexColor = aColor;
    normal = aNormal;
    ao = aAO;
}
