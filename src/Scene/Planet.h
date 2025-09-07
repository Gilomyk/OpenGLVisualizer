#pragma once

#include "../GL/Mesh.h"
#include "../GL/SphereGen.h"
#include "../Shader.h"
#include "../Renderer.h"
#include "../Core/Camera.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Planet {
private:
    SphereGen m_Sphere;
    VertexBufferLayout m_Layout;
    Mesh m_Mesh;
    glm::vec3 m_Translation;
    glm::vec3 m_Scale;
    glm::vec3 m_Rotation; // w stopniach (Euler)

public:
    Planet(float radius, unsigned int sectorCount, unsigned int stackCount, Texture* texture);

    void SetPosition(const glm::vec3& pos) { m_Translation = pos; }
    void SetScale(const glm::vec3& scale) { m_Scale = scale; }
    void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; }

    glm::vec3& GetPosition() { return m_Translation; }
    glm::vec3& GetScale() { return m_Scale; }
    glm::vec3& GetRotation() { return m_Rotation; }

    void Draw(Shader& shader, Renderer& renderer, const Camera& camera);
};
