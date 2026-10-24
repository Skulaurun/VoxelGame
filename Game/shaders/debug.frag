#version 450

out vec4 fragment;

in vec2 uv;

uniform sampler2D uDepth;

void main()
{
    float depth = texture(uDepth, uv).r;
    fragment = vec4(vec3(depth), 1.0);
}
