#pragma once

#include <vector>
#include "../VertexBuffer.h"
#include "../VertexArray.h"
#include "../VertexBufferLayout.h"
#include "../IndexBuffer.h"
#include "../Texture.h"

class Mesh {
	VertexArray m_VAO;
	VertexBuffer m_VBO;
	IndexBuffer m_IBO;
	VertexBufferLayout m_Layout;
	Texture* m_Texture;

public:
	Mesh(const float* vertices, unsigned int vertexCount,
		const unsigned int* indices, unsigned int indexCount,
		const VertexBufferLayout& layout, Texture* texture = nullptr);

	void Bind() const;
	void Unbind() const;

	unsigned int GetIndexCount() const ;
	Texture* GetTexture() const ;
};
