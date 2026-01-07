#pragma once

#include <vector>
#include "../Graphics/Buffers/VertexBuffer.h"
#include "../Graphics/Buffers/VertexArray.h"
#include "../Graphics/Buffers/VertexBufferLayout.h"
#include "../Graphics/Buffers/IndexBuffer.h"
#include "../Graphics/Texture.h"

class Mesh {
	VertexArray m_VAO;
	VertexBuffer m_VBO;
	IndexBuffer m_IBO;
	VertexBufferLayout m_Layout;

public:

	Mesh();

	Mesh(const float* vertices, unsigned int vertexCount,
		const unsigned int* indices, unsigned int indexCount,
		const VertexBufferLayout& layout);

	void Bind() const;
	void Unbind() const;

	unsigned int GetIndexCount() const ;
};
