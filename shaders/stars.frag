#version 330 core
out vec4 FragColor;

in float starSeed;

uniform float uTime;

uniform float uFlickerScale; // 0.0 - 1.0 z AudioMapper
uniform float uAtmosphereAlpha; // 0.0 - 1.0 z AudioMapper

void main()
{
    // ---- miękka maska punktu ----
    vec2 uv = gl_PointCoord - vec2(0.5);
    if (dot(uv, uv) > 0.25) discard;

    // ---- miganie ----
    float freq = 0.5 + fract(sin(starSeed * 91.7) * 43758.5453) * 2.0;
    float phase = fract(sin(starSeed * 13.1) * 15731.5) * 6.2831;

    // sinusoida [0..1]
    float flicker = 0.5 + 0.5 * sin(uTime * freq + phase);


    // ograniczenie migania
    float brightness = mix(0.5, 1.5, flicker * (0.5 + 0.5 * uFlickerScale));

    FragColor = vec4(vec3(brightness), flicker); // alfa jako miganie
}