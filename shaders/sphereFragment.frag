#version 330 core
in vec2 v_texCoord; // Texture coordinates passed from the vertex shader

uniform sampler2D uTexture;

out vec4 FragColor;

void main()
{
    // Sample the texture at the current fragment's coordinates
    vec4 texColor = texture(uTexture, v_texCoord);
    FragColor = texColor;
    // FragColor = vec4(1,0,0,1);
}
