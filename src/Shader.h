#pragma once

#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

class Shader {
private:
	std::string m_VertexPath;
	std::string m_FragmentPath;
	unsigned int m_RendererID;
public:
	Shader(const std::string& vertexPath, const std::string& fragmentPath);
	~Shader();
	void Bind() const;
	void Unbind() const;

	//Set uniforms
	void SetUniform1i(const std::string& name, int value) const ;
	void SetUniform1f(const std::string& name, float value);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
private:
	std::string LoadShaderSource(const std::string& path);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexSource, const std::string& fragmentSource);

	unsigned int GetUniformLocation(const std::string& name) const;
	mutable std::unordered_map<std::string, int> m_UniformLocationCache;
};
