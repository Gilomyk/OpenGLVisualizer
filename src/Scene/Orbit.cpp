#include "Orbit.h"
#include <GLFW/glfw3.h>

Orbit::Orbit(float radius, int segments, glm::vec3 orbitColor)
	: m_Orbit(radius, segments),
	m_Layout([] {
		VertexBufferLayout l;
		l.Push<float>(3); // position
		return l;
		}()), 
	m_Mesh(m_Orbit.GetVertices().data(),
		static_cast<unsigned int>(m_Orbit.GetVertices().size()),
		m_Orbit.GetIndices().data(),
		static_cast<unsigned int>(m_Orbit.GetIndices().size()),
		m_Layout),
	m_Translation(0.0f),
	m_Scale(1.0f),
	m_Rotation(0.0f, 0.0f, 0.0f)
{   
	m_OrbitColor = orbitColor;
}

void Orbit::DrawOrbit(Shader& shader, Renderer& renderer, const Camera& camera) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, m_Translation);
	model = glm::scale(model, m_Scale);
	model *= glm::toMat4(glm::quat(glm::radians(m_Rotation)));

	shader.Bind();
	shader.SetUniformMat4f("uModel", model);
	shader.SetUniformMat4f("uView", camera.GetViewMatrix());
	shader.SetUniformMat4f("uProjection", camera.GetProjectionMatrix());

	shader.SetUniform3fv("uColor", m_OrbitColor);

	renderer.Draw(m_Mesh, shader, GL_LINE_LOOP);
}