#version 330 core
in vec2 vUV;
uniform sampler2D uDiffuseMap;
uniform vec3 uEmissiveColor; // np. ¿ó³to-pomarañczowy

out vec4 FragColor;

void main()
{
    vec3 texColor = texture(uDiffuseMap, vUV).rgb;
    FragColor = vec4(texColor * uEmissiveColor, 1.0);
}
