#version 450

out vec2 uv;

void main()
{
    // [0, 0]
    // [2, 0]
    // [0, 2]
    vec2 position = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);

    // [-1, -1]
    // [3, -1]
    // [-1, 3]
    gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);

    uv = position;
}
