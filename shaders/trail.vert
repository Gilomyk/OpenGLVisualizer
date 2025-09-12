#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 vPosWorld;

void main()
{
    vPosWorld = vec3(u_Model * vec4(aPos, 1.0));
    gl_Position = u_Projection * u_View * vec4(vPosWorld, 1.0);
}

