#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 texCoord;

out vec2 v_texCoord;

uniform mat4 u_MVP;

void main()
{
    gl_Position = u_MVP * vec4(aPos, 0.0f, 1.0);
    v_texCoord = texCoord;
}
