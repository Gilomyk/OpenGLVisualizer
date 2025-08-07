#pragma once
#include "VertexBuffer.h"
class VertexArray {
private:
	unsigned int m_RendererID;
public:
	VertexArray();
	~VertexArray();
	void Bind() const;
	void Unbind() const;
	unsigned int GetRendererID() const;
	void AddBuffer(const VertexBuffer& vb, unsigned int componentCount);
};
