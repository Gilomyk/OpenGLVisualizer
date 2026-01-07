#pragma once
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../Graphics/Shader.h"
#include "../Graphics/Renderer.h"
#include "../Core/Camera.h"

class OrbitTrail {
public:
    OrbitTrail();
    ~OrbitTrail();

    // dodaje punkt (np. pozycja planety)
    void AddPoint(const glm::vec3& pos);

    // czyœci œlad (np. przy resecie symulacji)
    void Clear();

    // rysowanie punktów albo linii
    void Draw(Shader& shader, Renderer& renderer, const Camera& camera, bool lineMode = false);

private:
    void UpdateBuffer();

    unsigned int m_VAO, m_VBO;
    std::vector<glm::vec3> m_Points;
    bool m_BufferDirty; // flaga – czy trzeba zaktualizowaæ VBO
};
