#include "Stars.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>

Stars::Stars(unsigned int count, float radius) : m_Count(count), m_VBO(nullptr) {
	std::vector<glm::vec3> positions;
	positions.reserve(count);
	for (unsigned int i = 0; i < count; ++i) {
		float theta = ((float)rand() / RAND_MAX) * 2.0f * 3.1415926f;
		float phi = acos(2.0f * ((float)rand() / RAND_MAX) - 1.0f);

		float x = radius * sin(phi) * cos(theta);
		float y = radius * sin(phi) * sin(theta);
		float z = radius * cos(phi);

		positions.push_back(glm::vec3(x, y, z));
	}
	m_VBO = new VertexBuffer(positions.data(), positions.size() * sizeof(glm::vec3));
	VertexBufferLayout layout;
	layout.Push<float>(3); // pozycje (x, y, z)
	m_VAO.AddBuffer(*m_VBO, layout);
}

void Stars::Draw(Shader& shader, const Camera& camera) {
	m_VAO.Bind();

	shader.Bind();
	shader.SetUniformMat4f("uView", camera.GetViewMatrix());
	shader.SetUniformMat4f("uProjection", camera.GetProjectionMatrix());

	shader.SetUniform1f("uTime", (float)glfwGetTime());
	std::cout << "uTime = " << (float)glfwGetTime() << std::endl;

	glDrawArrays(GL_POINTS, 0, m_Count);
}