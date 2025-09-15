#version 330 core
in vec3 vPosWorld;
in vec3 vNormal;
in vec2 vUV;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material uMaterial;

uniform bool uUseTexture;
uniform sampler2D uDiffuseMap;
uniform sampler2D uSpecularMap;

uniform vec3 uLightAmbient;
uniform vec3 uLightPos;
uniform vec3 uLightDiffuse;
uniform vec3 uViewPos;
uniform vec3 uLightSpecular;

out vec4 FragColor;

// Simple hash noise for subtle variation
float hashNoise(vec2 p) {
    return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    // === Base color (diffuse source) ===
    vec3 baseColor = uUseTexture ? texture(uDiffuseMap, vUV).rgb : uMaterial.diffuse;

    // Subtle noise modulation
    float n = hashNoise(vUV * 50.0);
    baseColor *= mix(0.85, 1.0, n); // same as (0.85 + 0.15 * n)

    // === Lighting ===
    // Ambient
    vec3 ambient = baseColor * uMaterial.ambient * uLightAmbient;

    // Diffuse
    vec3 lightDir = normalize(uLightPos - vPosWorld);
    float diff = max(dot(vNormal, lightDir), 0.0);
    vec3 diffuse = diff * baseColor * uLightDiffuse;

    // Specular
    vec3 viewDir = normalize(uViewPos - vPosWorld);
    vec3 reflectDir = reflect(-lightDir, vNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shininess);

    float specStrength = uUseTexture ? 1.0 : texture(uSpecularMap, vUV).r;
    vec3 specular = specStrength * spec * uMaterial.specular * uLightSpecular;

    // === Final color ===
    vec3 result = ambient + diffuse + specular;
    result = clamp(result, 0.0, 1.0); // ensure no overbright
    FragColor = vec4(result, 1.0);
}
