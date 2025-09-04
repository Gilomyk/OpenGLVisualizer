#pragma once

#ifndef SPHEREGEN_H
#define SPHEREGEN_H

#include <vector>
#include <cmath>
#include <glm/glm.hpp>

class SphereGen {
public:
	SphereGen(float radius, unsigned int sectorCount, unsigned int stackCount);

	const std::vector<float>& GetVertices() const { return vertices; }
	const std::vector<unsigned int>& GetIndices() const { return indices; }

private:
	float radius;
	unsigned int sectorCount;
	unsigned int stackCount;

	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	void generateSphere();
	
	std::vector<float> getVertices() {
		return vertices;
	}
	std::vector<unsigned int> getIndices() {
		return indices;
	}
};

#endif // SPHEREGEN_H

