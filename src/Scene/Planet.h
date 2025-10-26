#pragma once

#include "../GL/Mesh.h"
#include "../GL/SphereGen.h"
#include "../Shader.h"
#include "../Renderer.h"
#include "../Material.h"
#include "Orbit.h"
#include "../Core/Camera.h"
#include "../GL/OrbitTrail.h"

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

    glm::vec3 m_Position;
    glm::vec3 m_Scale;
    glm::vec3 m_Rotation; // w stopniach (Euler)

    float m_OrbitRadius;    // Odleg³oœæ od rodzica
    glm::vec3 m_OrbitTilt; // Nachylenie orbity wzglêdem p³aszczyzny XY
    float m_OrbitSpeed;     // Prêdkoœæ obrotu wokó³ rodzica
    float m_SpinSpeed;      // Prêdkoœæ obrotu w³asnego
    float m_OrbitAngle;     // Aktualny k¹t
    float m_SpinAngle;      // Aktualny k¹t rotacji w³asnej
    Planet* m_Parent;       // WskaŸnik na planetê-rodzica

    std::unique_ptr<OrbitTrail> m_Trail;
	std::unique_ptr<Orbit> m_Orbit;  // Unikalny wskaŸnik na obiekt orbity (jeœli istnieje)

    Material m_Material;
    Texture* m_Texture = nullptr;

public:
    Planet(float radius, unsigned int sectorCount, unsigned int stackCount, 
        float orbitRadius, glm::vec3 orbitTilt, float orbitSpeed, float spinSpeed, 
        Planet* parent = nullptr,
        bool enableOrbit = false);
    
	// Settery
    void SetPosition(const glm::vec3& pos) { m_Position = pos; }
    void SetScale(const glm::vec3& scale) { m_Scale = scale; }
    void SetRotation(const glm::vec3& rotation) { m_Rotation = rotation; }

	void SetOrbitRadius(float radius) { m_OrbitRadius = radius; }
    void SetOrbitTilt(const glm::vec3& tilt) { m_OrbitTilt = tilt; }
	void SetOrbitSpeed(float speed) { m_OrbitSpeed = speed; }
	void SetSpinSpeed(float speed) { m_SpinSpeed = speed; }
	void SetOrbitAngle(float angle) { m_OrbitAngle = angle; }
	void SetSpinAngle(float angle) { m_SpinAngle = angle; }

    void SetTexture(Texture* texture) { m_Texture = texture; }
    void SetMaterial(Material& material) { m_Material = material; }

	// Gettery
    glm::vec3& GetPosition() { return m_Position; }
    glm::vec3& GetScale() { return m_Scale; }
    glm::vec3& GetRotation() { return m_Rotation; }

	float GetOrbitRadius() const { return m_OrbitRadius; }
	glm::vec3 GetOrbitTilt() const { return m_OrbitTilt; }
	float GetOrbitSpeed() const { return m_OrbitSpeed; }
	float GetSpinSpeed() const { return m_SpinSpeed; }
	float GetOrbitAngle() const { return m_OrbitAngle; }
	float GetSpinAngle() const { return m_SpinAngle; }

    Planet* GetParent() { return m_Parent; }
    std::unique_ptr<OrbitTrail>& GetTrail() { return m_Trail; }
	std::unique_ptr<Orbit>& GetOrbit() { return m_Orbit; }

	// Aktualizacja
    void Update(float dt);  // Aktualizuje pozycjê i rotacjê

	// Rysowanie
    void DrawPlanet(Shader& shader, Renderer& renderer, const Camera& camera);
    void DrawSun(Shader& shader, Renderer& renderer, const Camera& camera);
	void DebugPrint() const;
};
