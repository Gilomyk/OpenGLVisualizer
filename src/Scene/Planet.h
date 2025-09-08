#pragma once

#include "../GL/Mesh.h"
#include "../GL/SphereGen.h"
#include "../Shader.h"
#include "../Renderer.h"
#include "Orbit.h"
#include "../Core/Camera.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

class Planet {
private:
    SphereGen m_Sphere;
    VertexBufferLayout m_Layout;
    Mesh m_Mesh;

    glm::vec3 m_Translation;
    glm::vec3 m_Scale;
    glm::vec3 m_Rotation; // w stopniach (Euler)

    float m_OrbitRadius;    // Odleg³oœæ od rodzica
    float m_OrbitSpeed;     // Prêdkoœæ obrotu wokó³ rodzica
    float m_SpinSpeed;      // Prêdkoœæ obrotu w³asnego
    float m_OrbitAngle;     // Aktualny k¹t
    float m_SpinAngle;      // Aktualny k¹t rotacji w³asnej
	float m_InitialAngle;  // K¹t pocz¹tkowy na orbicie

    Planet* m_Parent;       // WskaŸnik na planetê-rodzica

	std::unique_ptr<Orbit> m_Orbit;  // Unikalny wskaŸnik na obiekt orbity (jeœli istnieje)
	glm::vec3 m_OrbitTilt; // Nachylenie orbity wzglêdem p³aszczyzny XY

public:
    Planet(float radius, unsigned int sectorCount, unsigned int stackCount, float orbitRadius, glm::vec3 orbitTilt, float orbitSpeed, float spinSpeed, Planet* parent = nullptr, Texture* texture = nullptr, bool enableOrbit = false);

    void SetPosition(const glm::vec3& pos) { m_Translation = pos; }
    void SetScale(const glm::vec3& scale) { m_Scale = scale; }
    void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; }

    glm::vec3& GetPosition() { return m_Translation; }
    glm::vec3& GetScale() { return m_Scale; }
    glm::vec3& GetRotation() { return m_Rotation; }

	std::unique_ptr<Orbit>& GetOrbit() { return m_Orbit; }

    void Update(float dt);  // Aktualizuje pozycjê i rotacjê

    void DrawPlanet(Shader& shader, Renderer& renderer, const Camera& camera);
    void DrawSun(Shader& shader, Renderer& renderer, const Camera& camera);
};
