#include "VertexArray.h"
#include "Renderer.h"
#include "VertexBuffer.h"

VertexArray::VertexArray() {
	GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray() {
	GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::Bind() const {
	GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() const {
	GLCall(glBindVertexArray(0));
}

unsigned int VertexArray::GetRendererID() const {
	return m_RendererID;
}

void VertexArray::AddBuffer(const VertexBuffer& vb, unsigned int componentCount) {
	Bind();
	vb.Bind();
	GLCall(GLCall(glEnableVertexAttribArray(0)));
	GLCall(GLCall(glVertexAttribPointer(0, componentCount, GL_FLOAT, GL_FALSE, componentCount * sizeof(float), (const void*)0)));
	// Unbind the VAO to prevent accidental modifications
	Unbind();
	vb.Unbind();
}