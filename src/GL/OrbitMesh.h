#pragma once
#include <vector>

struct OrbitData {
    std::vector<float> vertices;   // po 3 floaty na wierzcho³ek
    std::vector<unsigned int> indices;
};

OrbitData GenerateOrbit(float radius, int segments) {
	const float PI = 3.14159265359f;
    OrbitData data;
    data.vertices.reserve(segments * 3);
    data.indices.reserve(segments);

    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        data.vertices.push_back(x);
        data.vertices.push_back(0.0f);
        data.vertices.push_back(z);
        data.indices.push_back(i);
    }
    return data;
}