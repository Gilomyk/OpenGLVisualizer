#version 330 core
in vec3 vPosWorld; // Position passed from the vertex shader
in vec3 vNormal; // Normal vector passed from the vertex shader
in vec2 vUV; // Texture coordinates passed from the vertex shader

uniform sampler2D uDiffuseMap;
uniform sampler2D uSpecularMap;
uniform bool hasDiffuse;
uniform bool hasSpecular;
uniform vec3 materialColor;   // fallback, gdy brak diffuse
uniform float uShininess;

uniform vec3 uLightAmbient;
uniform vec3 uLightPos;
uniform vec3 uLightDiffuse;

uniform vec3 uViewPos; // Camera position
uniform vec3 uLightSpecular; // Specular light color

out vec4 FragColor;

// simple hash noise
float hashNoise(vec2 p) {
    return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}


void main()
{
    // === Base color (diffuse source) ===
    vec3 baseColor;
    if (hasDiffuse) {
        baseColor = texture(uDiffuseMap, vUV).rgb;
    } else {
        baseColor = materialColor;
    }

    // add simple noise modulation
    float n = hashNoise(vUV * 50.0);
    baseColor *= (0.85 + 0.15 * n);

    // === Lighting ===
    // Ambient
    vec3 ambient = uLightAmbient * baseColor;

    // Diffuse
    vec3 lightDir = normalize(uLightPos - vPosWorld);
    float diff = max(dot(vNormal, lightDir), 0.0);
    vec3 diffuse = diff * uLightDiffuse * baseColor;

    // Specular
    vec3 viewDir = normalize(uViewPos - vPosWorld);
    vec3 reflectDir = reflect(-lightDir, vNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);

    float specStrength = 1.0;
    if (hasSpecular) {
        specStrength = texture(uSpecularMap, vUV).r;
    }

    vec3 specular = specStrength * spec * uLightSpecular;

    // Final
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
