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
	void SetUniform3f(const std::string& name, float v0, float v1, float v2);
	void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
	void SetUniform2fv(const std::string& name, const glm::vec2& vec);
	void SetUniform3fv(const std::string& name, const glm::vec3& vec);
	void SetUniformMat3f(const std::string& name, const glm::mat3& matrix);
	void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
private:
	std::string LoadShaderSource(const std::string& path);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexSource, const std::string& fragmentSource);

	unsigned int GetUniformLocation(const std::string& name) const;
	mutable std::unordered_map<std::string, int> m_UniformLocationCache;
};
