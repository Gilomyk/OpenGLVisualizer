#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uView;
uniform mat4 uProjection;
uniform float uFlickerScale; // 0..1 z AudioMapper
uniform float uTime;


out float starSeed;

void main()
{
    gl_Position = uProjection * uView * vec4(aPos, 1.0);
    starSeed = float(gl_VertexID);

    float freq = 0.5 + fract(sin(starSeed * 91.7) * 43758.5453) * 2.0;
    float phase = fract(sin(starSeed * 13.1) * 15731.5) * 6.2831;

    float baseSize = 1.5 + fract(sin(starSeed * 23.7) * 6789.123) * 2.5;
    float pulse = 0.5 + 0.5 * sin(uTime * freq + phase);

    float size = baseSize * mix(1.0, 1.0 + 2.5 * uFlickerScale, pulse);

    gl_PointSize = size;
}