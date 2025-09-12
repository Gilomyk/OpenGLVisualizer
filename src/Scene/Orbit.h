#pragma once
#include "../GL/Mesh.h"
#include "../GL/OrbitGen.h"
#include "../Shader.h"
#include "../Renderer.h"
#include "../Core/Camera.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Orbit {
private:
    OrbitGen m_Orbit;
    VertexBufferLayout m_Layout;
    Mesh m_Mesh;
    glm::vec3 m_Position;
    glm::vec3 m_Scale;
    glm::vec3 m_Rotation;
	// w stopniach (Euler)
	glm::vec3 m_OrbitColor;

public:
    Orbit(float radius, int segments, glm::vec3 orbitColor);

    void SetPosition(const glm::vec3& pos) { m_Position = pos; }
    void SetScale(const glm::vec3& scale) { m_Scale = scale; }
    void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; }

    glm::vec3& GetPosition() { return m_Position; }
    glm::vec3& GetScale() { return m_Scale; }
    glm::vec3& GetRotation() { return m_Rotation; }

    void DrawOrbit(Shader& shader, Renderer& renderer, const Camera& camera, const glm::vec3& parentPos);
};