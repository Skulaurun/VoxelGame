#version 450

in vec4 vertexColor;
in float ao;

out vec4 fragment;

void main()
{
    vec4 color = vertexColor;
    color.xyz *= min(ao + 0.25, 1.0);

    fragment = color;
}
