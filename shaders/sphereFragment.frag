#version 330 core
in vec3 vPosWorld; // Position passed from the vertex shader
in vec3 vNormal; // Normal vector passed from the vertex shader
in vec2 vUV; // Texture coordinates passed from the vertex shader

uniform sampler2D uTexture;

uniform vec3 uLightAmbient;
uniform vec3 uLightPos;
uniform vec3 uLightDiffuse;

uniform vec3 uViewPos; // Camera position
uniform vec3 uLightSpecular; // Specular light color
uniform float uShininess; // Shininess factor for specular highlight

out vec4 FragColor;

void main()
{
    // Texture color
    vec3 texColor = vec3(texture(uTexture, vUV));

    // Ambient component
    vec3 ambient = uLightAmbient * texColor;

    // Diffuse component
    vec3 lightDir = normalize(uLightPos - vPosWorld);
    float diff = max(dot(vNormal, lightDir), 0.0);
    vec3 diffuse = diff * uLightDiffuse * texColor;

    // Specular component
    vec3 viewDir = normalize(uViewPos - vPosWorld);
    vec3 reflectDir = reflect(-lightDir, vNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), uShininess);
    vec3 specular = spec * uLightSpecular * texColor;

    // Combine all components
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);

    // Test with no lightning
//    vec4 texColor = texture(uTexture, vUV);
//    FragColor = texColor;
}
