#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include "Renderer.h"

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
	: m_VertexPath(vertexPath), m_FragmentPath(fragmentPath), m_RendererID(0)
{
    std::string vertexSource = LoadShaderSource(vertexPath);
    std::string fragmentSource = LoadShaderSource(fragmentPath);
    m_RendererID = CreateShader(vertexSource, fragmentSource);
}

Shader::~Shader() {
    GLCall(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const {
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const {
    GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int value) const {
	GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1f(const std::string& name, float value) {
	GLCall(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2) {
	GLCall(glUniform3f(GetUniformLocation(name), v0, v1, v2));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniform2fv(const std::string& name, const glm::vec2& vec) {
    GLCall(glUniform2fv(GetUniformLocation(name), 1, &vec[0]));
}

void Shader::SetUniform3fv(const std::string& name, const glm::vec3& vec) {
	GLCall(glUniform3fv(GetUniformLocation(name), 1, &vec[0]));
}

void Shader::SetUniformMat3f(const std::string& name, const glm::mat3& matrix) {
	GLCall(glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) {
	GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

unsigned int Shader::GetUniformLocation(const std::string& name) const {
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    m_UniformLocationCache[name] = location;
    return location;
}


std::string Shader::LoadShaderSource(const std::string& path) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Błąd?
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, NULL, info);
        std::cerr << "Shader error: " << info << std::endl;
    }
    return shader;
}

unsigned int Shader::CreateShader(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    // Sprawdź błędy linkowania
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}