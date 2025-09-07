#include "OrbitGen.h"
#include <cmath>

OrbitGen::OrbitGen(float radius, int segments)
	: radius(radius), segments(segments) {
	generateOrbit();
}

// Funkcja rysuj¹ca orbitê jako okr¹g z³o¿ony z linii
void OrbitGen::generateOrbit() {
	const float PI = 3.14159265359f;
	vertices.clear();
	indices.clear();
	for (int i = 0; i < segments; i++) {
		float angle = 2.0f * PI * i / segments;
		float x = radius * cos(angle);
		float z = radius * sin(angle);
		vertices.push_back(x);
		vertices.push_back(0.0f);
		vertices.push_back(z);
		indices.push_back(i);
	}
}