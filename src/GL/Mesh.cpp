#include "Mesh.h"

Mesh::Mesh()
    : m_VBO(nullptr, 0), m_IBO(nullptr, 0), m_Texture(nullptr)
{ }

Mesh::Mesh(const float* vertices, unsigned int vertexCount,
    const unsigned int* indices, unsigned int indexCount,
    const VertexBufferLayout& layout, Texture* texture)
    : m_VBO(vertices, vertexCount * sizeof(float)),
    m_IBO(indices, indexCount),
    m_Layout(layout),
    m_Texture(texture)
{
    std::cout << "Mesh constructor start" << std::endl;
    std::cout << "VBO ID: " << m_VBO.GetRendererID() << std::endl;
    std::cout << "IBO ID: " << m_IBO.GetRendererID() << std::endl;

    m_VAO.AddBuffer(m_VBO, m_Layout);

    std::cout << "VAO ID after AddBuffer: " << m_VAO.GetRendererID() << std::endl;
    std::cout << "Mesh constructor end" << std::endl;
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