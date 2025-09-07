#pragma once

#include <vector>
#include <cmath>
#include <glm/glm.hpp>

class OrbitGen {
public:
	OrbitGen(float radius, int segments);
	const std::vector<float>& GetVertices() const { return vertices; }
	const std::vector<unsigned int>& GetIndices() const { return indices; }

private:
	float radius;
	int segments;

	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	void generateOrbit();

	std::vector<float> getVertices() {
		return vertices;
	}
	std::vector<unsigned int> getIndices() {
		return indices;
	}
};