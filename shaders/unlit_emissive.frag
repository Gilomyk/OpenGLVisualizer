#version 330 core

in vec2 vUV;
uniform float uTime;           // aktualny czas w sekundach
uniform vec3 uBaseColor;       // bazowy kolor gwiazdy (np. glm::vec3(1.0, 0.9, 0.3))
uniform float uFlickerStrength; // np. 0.2
uniform float uGradientFalloff; // np. 1.5 (kontrola radialnego spadku)

out vec4 FragColor;

// prosty hash noise zale¿ny od UV
float hashNoise(vec2 p)
{
    return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}

// Prosty 2D Perlin Noise (przyk³ad)
float fade(float t) { return t*t*t*(t*(t*6-15)+10); }
float lerp(float a, float b, float t) { return a + t*(b-a); }
float grad(int hash, float x, float y) { 
    int h = hash & 3; 
    float u = h<2 ? x : y;
    float v = h<2 ? y : x;
    return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}
float perlin(vec2 P) {
    vec2 Pi = floor(P);
    vec2 Pf = P - Pi;
    int A = int(Pi.x + Pi.y*57.0);
    int B = int(Pi.x+1.0 + Pi.y*57.0);
    float u = fade(Pf.x);
    float v = fade(Pf.y);
    return lerp(
        lerp(grad(A, Pf.x, Pf.y), grad(B, Pf.x-1.0, Pf.y), u),
        lerp(grad(A+57, Pf.x, Pf.y-1.0), grad(B+57, Pf.x-1.0, Pf.y-1.0), u),
        v
    );
}

void main()
{
    // === odleg³oœæ od œrodka sfery (0.5,0.5 w UV) ===
    vec2 uvCentered = vUV - vec2(0.5);
    float dist = length(uvCentered);

    // === gradient radialny ===
    vec3 radialColor = mix(uBaseColor, uBaseColor * 0.2, pow(dist, uGradientFalloff));

    // === losowy puls / fluktuacja ===
    float n = hashNoise(vUV * 50.0 + vec2(uTime*10.0, uTime*10.0));
    float flicker = 1.0 + (n - 0.5) * 2.0 * uFlickerStrength;

    // === dodatkowe kolory / „plamy” ===
    // przyk³ad z hash noise
    float spots = hashNoise(vUV * 200.0 + vec2(sin(uTime*0.7), cos(uTime*0.5)));
    radialColor *= (0.9 + 0.2 * spots);

    // === finalny kolor ===
    vec3 finalColor = radialColor * flicker;

    FragColor = vec4(finalColor, 1.0);
}
