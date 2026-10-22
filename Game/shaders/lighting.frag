#version 450

out vec4 fragment;

in vec2 uv;

uniform sampler2D uNormal;
uniform sampler2D uAlbedo;
uniform sampler2D uDepth;

uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform float uSunIntensity;
uniform vec3 uAmbientColor;

void main()
{
    float depth = texture(uDepth, uv).r;

    if (depth >= 1.0) {
        // sky color
        fragment = vec4(0.529, 0.808, 0.922, 1.0);
        return;
    }

    vec3 normal = texture(uNormal, uv).rgb;
    vec3 albedo = texture(uAlbedo, uv).rgb;

    float NdotL = max(dot(normal, uSunDirection), 0.0);
    vec3 directSunlight = uSunIntensity * uSunColor * NdotL;
    vec3 lighting = directSunlight + uAmbientColor;

    vec3 color = albedo * lighting;

    fragment = vec4(color, 1.0);
}
