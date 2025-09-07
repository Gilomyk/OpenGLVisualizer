#include "Planet.h"
#include "../Renderer.h"
#include <GLFW/glfw3.h>

Planet::Planet(float radius, unsigned int sectorCount, unsigned int stackCount, Texture* texture)
    : m_Sphere(radius, sectorCount, stackCount),
    m_Layout([] {
        VertexBufferLayout l;
        l.Push<float>(3); // position
        l.Push<float>(3); // normal
        l.Push<float>(2); // texcoord
        return l;
    }()),
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

}

void Planet::DrawPlanet(Shader& shader, Renderer& renderer, const Camera& camera) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_Translation);
    model = glm::scale(model, m_Scale);
    model *= glm::toMat4(glm::quat(glm::radians(m_Rotation)));

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    shader.Bind();
    shader.SetUniformMat4f("uModel", model);
    shader.SetUniformMat4f("uView", camera.GetViewMatrix());
    shader.SetUniformMat4f("uProjection", camera.GetProjectionMatrix());
    shader.SetUniformMat3f("uNormalMatrix", normalMatrix);

    shader.SetUniform3fv("uLightPos", glm::vec3(0.0f, 0.0f, 0.0f));
    shader.SetUniform3fv("uLightAmbient", glm::vec3(0.2f, 0.2f, 0.2f));
    shader.SetUniform3fv("uLightDiffuse", glm::vec3(0.7f, 0.7f, 0.7f));
    shader.SetUniform3fv("uLightSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

    shader.SetUniform3fv("uViewPos", camera.GetPosition());
    shader.SetUniform1f("uShininess", 32.0f);

    renderer.Draw(m_Mesh, shader);
}

void Planet::DrawSun(Shader& shader, Renderer& renderer, const Camera& camera) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_Translation);
    model = glm::scale(model, m_Scale);
    model *= glm::toMat4(glm::quat(glm::radians(m_Rotation)));

    shader.Bind();
    shader.SetUniformMat4f("uModel", model);
    shader.SetUniformMat4f("uView", camera.GetViewMatrix());
    shader.SetUniformMat4f("uProjection", camera.GetProjectionMatrix());

    shader.SetUniform3f("uEmissiveColor", 1.0f, 0.9f, 0.3f);

    renderer.Draw(m_Mesh, shader);
}