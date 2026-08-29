#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;
layout (location = 4) in float aAO;

uniform mat4 uViewProjection;
out vec4 vertexColor;
out float ao;

void main()
{
    gl_Position = uViewProjection * vec4(aPosition, 1.0);
    
    vertexColor = aColor;
    ao = aAO;
}
