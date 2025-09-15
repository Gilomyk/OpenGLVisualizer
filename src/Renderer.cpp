#include "Renderer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Renderer.h"
#include "GL/Mesh.h"

#include <iostream>

void GLClearError() {
	while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError()) {
		std::cerr << "[OpenGL Error] (" << error << "): "
			<< function << " "
			<< file << ":" << line << std::endl;
		return false;
	}
	return true;
}

void Renderer::Clear() const {
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::Draw(const Mesh& mesh, const Shader& shader, GLenum primitiveType) const {
	mesh.Bind();
	GLCall(glDrawElements(primitiveType, mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr));
}