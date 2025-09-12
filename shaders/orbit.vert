#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vPosWorld;

void main()
{
    vPosWorld = vec3(uModel * vec4(aPos, 1.0));
    gl_Position = uProjection * uView * vec4(vPosWorld, 1.0);
}
