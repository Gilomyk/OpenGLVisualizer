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

uniform float uTextureBlend;
uniform sampler2D uDiffuseMap;
uniform sampler2D uSpecularMap;

uniform vec3 uLightAmbient;
uniform vec3 uLightPos;
uniform vec3 uLightDiffuse;
uniform vec3 uViewPos;
uniform vec3 uLightSpecular;

// audio uniforms
uniform float uSpecularScale;  // skaluje specular
uniform float uNoiseAmount;    // 0..1
uniform float uAtmosphereAlpha; // 0..1

out vec4 FragColor;

// Simple hash noise for subtle variation
float hashNoise(vec2 p) {
    return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec3 texColor = texture(uDiffuseMap, vUV).rgb;
    vec3 matColor = uMaterial.diffuse;

    // === Base color blending ===
    vec3 baseColor = mix(matColor, texColor, uTextureBlend);

    // Subtle noise modulation
    float n = hashNoise(vUV * 50.0);
    float noiseAmp = mix(0.0, 0.15, uNoiseAmount); // 0..0.15 max modulation
    baseColor *= (1.0 - noiseAmp) + noiseAmp * n;   // = 1.0 - noiseAmp + noiseAmp*n

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

    // Adjust shininess based on texture blend
    float shininessAdj = mix(uMaterial.shininess, uMaterial.shininess * 0.6, uTextureBlend);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininessAdj);
    
    // Specular map influence
    float specMapVal = texture(uSpecularMap, vUV).b;
    float specStrength = mix(1.0, specMapVal, uTextureBlend);

    vec3 specularColor = mix(vec3(1.0), uMaterial.specular, 0.7);
    vec3 specular = specStrength * spec * specularColor * (0.2 + 2.5 * uSpecularScale);
    // vec3 specular = vec3(0.0);

    // === Final color ===
    vec3 result = (ambient + diffuse + specular);
    result = mix(result, result * 1.6, uAtmosphereAlpha); // blend to white for atmosphere
    // result = clamp(result, 0.0, 1.0); // ensure no overbright
    FragColor = vec4(result, 1.0);
}
