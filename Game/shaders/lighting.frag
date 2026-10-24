#version 450

out vec4 fragment;

in vec2 uv;

uniform sampler2D uNormal;
uniform sampler2D uAlbedo;
uniform sampler2D uDepth;

uniform sampler2D uShadowMap;

uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform float uSunIntensity;
uniform vec3 uAmbientColor;
uniform vec3 uSkyColor;

uniform mat4 uViewProjection;
uniform mat4 uLightViewProjection;

vec3 worldPosition(vec2 uv, float depth, mat4 inverseMatrix) {
    vec4 ndc = vec4(
        uv * 2.0 - 1.0,
        depth * 2.0 - 1.0,
        1.0
    );
    
    // homogenous coordinates
    vec4 position = inverseMatrix * ndc;

    return position.xyz / position.w;
}

float pcf(vec2 uv, float currentDepth, float bias) {
    float shadow = 0;
    // one texel in UV coordinates [0, 1]
    vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float depth = texture(uShadowMap, uv + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > depth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}

void main()
{
    float depth = texture(uDepth, uv).r;

    if (depth >= 1.0) {
        fragment = vec4(uSkyColor, 1.0);
        return;
    }

    vec3 normal = texture(uNormal, uv).rgb;
    vec3 albedo = texture(uAlbedo, uv).rgb;

    float NdotL = dot(normal, uSunDirection);

    vec3 position = worldPosition(uv, depth, inverse(uViewProjection));
    vec4 lightSpacePosition = uLightViewProjection * vec4(position, 1.0);
    lightSpacePosition.xyz /= lightSpacePosition.w;

    vec3 lightUvw = lightSpacePosition.xyz * 0.5 + 0.5;

    float bias = max(0.005 * (1.0 - NdotL), 0.0005);
    float shadow = 0.0;
    // TODO: set texparameteri to clamp to border and set border to no shadow
    if (all(greaterThanEqual(lightUvw, vec3(0.0))) && all(lessThanEqual(lightUvw, vec3(1.0)))) {
        // float lightDepth = texture(uShadowMap, lightUvw.xy).r;
        // shadow = lightDepth < lightUvw.z - bias ? 1.0 : 0.0;
        shadow = pcf(lightUvw.xy, lightUvw.z, bias);
    }

    vec3 directSunlight = uSunIntensity * uSunColor * max(NdotL, 0.0);
    vec3 lighting = (1.0 - shadow) * directSunlight + uAmbientColor;

    vec3 color = albedo * lighting;

    fragment = vec4(color, 1.0);
}
