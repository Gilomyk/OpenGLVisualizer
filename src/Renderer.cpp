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

void Renderer::Draw(const Mesh& mesh, const Shader& shader) const {
	if (mesh.GetTexture())
	{
		mesh.GetTexture()->Bind();
        shader.SetUniform1i("uTexture", 0);
		std::cout << "Texture bound with ID: " << mesh.GetTexture()->GetWidth() << "x" << mesh.GetTexture()->GetHeight() << std::endl;
	}
	mesh.Bind();
	GLCall(glDrawElements(GL_TRIANGLES, mesh.GetIndexCount(), GL_UNSIGNED_INT, nullptr));
}