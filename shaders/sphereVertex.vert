#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 texCoord;

out vec3 vPosWorld;
out vec3 vNormal;
out vec2 vUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

uniform mat4 u_MVP;

void main()
{
    vPosWorld = vec3(uModel * vec4(aPos, 1.0));
    vNormal = normalize(uNormalMatrix * aNormal);
    vUV = texCoord;

    gl_Position = uProjection * uView * vec4(vPosWorld, 1.0);
//    gl_Position = u_MVP * vec4(aPos, 1.0);
}
