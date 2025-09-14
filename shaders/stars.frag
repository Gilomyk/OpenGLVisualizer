#version 330 core
out vec4 FragColor;

in float starSeed;

uniform float uTime;

void main()
{
    // ---- miêkka maska punktu ----
    vec2 uv = gl_PointCoord - vec2(0.5);
    if (dot(uv, uv) > 0.25) discard;

    // ---- miganie ----
    float freq = 0.5 + fract(sin(starSeed * 91.7) * 43758.5453) * 2.0;
    float phase = fract(sin(starSeed * 13.1) * 15731.5) * 6.2831;

    // sinusoida [0..1]
    float flicker = 0.5 + 0.5 * sin(uTime * freq + phase);

    // ogranicz wahania (¿eby nie gas³y ca³kiem)
    flicker = mix(0.3, 1.0, flicker);

    FragColor = vec4(1.0, 1.0, 1.0, flicker); // bia³y punkt
}