#include "Planet.h"
#include "../Renderer.h"
#include <GLFW/glfw3.h>

Planet::Planet(float radius, unsigned int sectorCount, unsigned int stackCount, Texture* texture)
    : m_Sphere(radius, sectorCount, stackCount),
	m_Layout(),
    m_Mesh(m_Sphere.GetVertices().data(),
        static_cast<unsigned int>(m_Sphere.GetVertices().size()),
        m_Sphere.GetIndices().data(),
        static_cast<unsigned int>(m_Sphere.GetIndices().size()),
        m_Layout,
        texture),
    m_Translation(0.0f),
    m_Scale(1.0f),
    m_Rotation(0.0f)
{
    m_Layout.Push<float>(3);
    m_Layout.Push<float>(3);
    m_Layout.Push<float>(2);
}

void Planet::Draw(Shader& shader, const Camera& camera) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_Translation);
    model = glm::scale(model, m_Scale);

    glm::quat q = glm::quat(glm::radians(m_Rotation));
    model *= glm::toMat4(q);

    glm::mat4 mvp = camera.GetProjectionMatrix() * camera.GetViewMatrix() * model;

    shader.Bind();
    shader.SetUniformMat4f("u_MVP", mvp);

    m_Mesh.Bind();
    glDrawElements(GL_TRIANGLES, m_Mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
}