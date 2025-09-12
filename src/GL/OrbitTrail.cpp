#include "OrbitTrail.h"
#include <glad/glad.h>

OrbitTrail::OrbitTrail()
    : m_VAO(0), m_VBO(0), m_BufferDirty(false)
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    // na start pusty buffer, póŸniej nadpisujemy
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);
}

OrbitTrail::~OrbitTrail()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}

void OrbitTrail::AddPoint(const glm::vec3& pos)
{
    m_Points.push_back(pos);
    m_BufferDirty = true;
}

void OrbitTrail::Clear()
{
    m_Points.clear();
    m_BufferDirty = true;
}

void OrbitTrail::UpdateBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_Points.size() * sizeof(glm::vec3), m_Points.data(), GL_DYNAMIC_DRAW);
    m_BufferDirty = false;
}

void OrbitTrail::Draw(Shader& shader, Renderer& renderer, const Camera& camera, bool lineMode)
{
    if (m_Points.empty()) return;

    if (m_BufferDirty) {
        UpdateBuffer();
    }

    glBindVertexArray(m_VAO);

    shader.Bind();
    shader.SetUniformMat4f("u_View", camera.GetViewMatrix());
    shader.SetUniformMat4f("u_Projection", camera.GetProjectionMatrix());
    shader.SetUniformMat4f("u_Model", glm::mat4(1.0f));

	shader.SetUniform3f("u_Color", 0.8f, 0.8f, 0.2f); // kolor œladu


    if (lineMode)
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)m_Points.size());
    else
        glDrawArrays(GL_POINTS, 0, (GLsizei)m_Points.size());
}
