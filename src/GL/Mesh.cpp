#include "Mesh.h"

Mesh::Mesh(const float* vertices, unsigned int vertexCount,
    const unsigned int* indices, unsigned int indexCount,
    const VertexBufferLayout& layout, Texture* texture)
    : m_VBO(vertices, vertexCount * sizeof(float)),
    m_IBO(indices, indexCount),
    m_Layout(layout),
    m_Texture(texture)
{
    m_VAO.AddBuffer(m_VBO, m_Layout);
}

void Mesh::Bind() const {
	m_VAO.Bind();
	m_IBO.Bind();
}

void Mesh::Unbind() const {
	m_VAO.Unbind();
	m_IBO.Unbind();
}

unsigned int Mesh::GetIndexCount() const {
	return m_IBO.GetCount();
}

Texture* Mesh::GetTexture() const {
	return m_Texture;
}