#include "SphereGen.h"
#include <cmath>

SphereGen::SphereGen(float radius, unsigned int sectorCount, unsigned int stackCount)
	: radius(radius), sectorCount(sectorCount), stackCount(stackCount) {
	generateSphere();
}

// Funkcja rysuj¹ca sferê UV
void SphereGen::generateSphere() {
	const float PI = 3.14159265359f;
	vertices.clear();
	indices.clear();

	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float u, v;									 // vertex texCoord

	for (unsigned int i = 0; i <= stackCount; ++i) {
		float stackAngle = PI / 2 - (float)i * PI / stackCount; // k¹t theta
		xy = radius * cosf(stackAngle);
		z = radius * sinf(stackAngle);

		for (unsigned int j = 0; j <= sectorCount; ++j) {
			float sectorAngle = (float)j * 2 * PI / sectorCount; // k¹t phi

			x = xy * cosf(sectorAngle);
			y = xy * sinf(sectorAngle);

			// Wspó³rzêdne wieerzcho³ków
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			// normalne (wektory jednostkowe)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			vertices.push_back(nx);
			vertices.push_back(ny);
			vertices.push_back(nz);

			// Wspó³rzêdne tekstury
			u = 1.0f - (float)j / sectorCount;
			v = (float)i / stackCount;
			vertices.push_back(u);
			vertices.push_back(v);
		}
	}

	// Indeksy
	for (unsigned int i = 0; i <= stackCount; ++i) {
		unsigned int k1 = i * (sectorCount + 1);
		unsigned int k2 = k1 + sectorCount + 1;

		for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
			if (i != 0) {
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}
			if (i != (stackCount - 1)) {
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}
}