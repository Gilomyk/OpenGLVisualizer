#include "CameraController.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <cstdlib>

void CameraController::Update(float dt, const std::vector<std::unique_ptr<Planet>>& planets, float localTempo)
{
    // Tempo  → wpływa na prędkość obrotu kamery
    m_TempoFactor = glm::mix(m_TempoFactor, localTempo * 0.4f + 0.6f, 0.1f);

    // === Jeśli kamera zablokowana na planecie ===
    if (m_IsLocked && m_TargetPlanet) {
        m_LockOnDuration -= dt;
        glm::vec3 target = m_TargetPlanet->GetPosition();

        glm::vec3 desiredPos = target + glm::vec3(0.0f, 150.0f, 250.0f);
        m_Camera->Position = glm::mix(m_Camera->Position, desiredPos, 0.02f);

        glm::vec3 newFront = glm::normalize(target - m_Camera->Position);
        m_Camera->Front = glm::mix(m_Camera->Front, newFront, 0.05f);
        m_Camera->updateCameraVectorsFront();

        if (m_LockOnDuration <= 0.0f) {
            m_IsLocked = false;
			m_ReturningToCenter = true;
        }
        return;
    }

    // === Swobodny ruch orbitalny ===
    m_OrbitAngle += m_OrbitSpeed * dt * (0.5f + m_TempoFactor);
    if (m_OrbitAngle > 360.0f) m_OrbitAngle -= 360.0f;

    float x = cos(glm::radians(m_OrbitAngle)) * m_OrbitRadius;
    float z = sin(glm::radians(m_OrbitAngle)) * m_OrbitRadius;
    float y = 150.0f + sin(glm::radians(m_OrbitAngle * 0.5f)) * 80.0f;

    glm::vec3 center = glm::vec3(0.0f);

	// === Jeśli kamera wraca do centrum ===
    if (m_ReturningToCenter) {
		glm::vec3 desiredPos = glm::vec3(x, y, z);
        m_Camera->Position = glm::mix(m_Camera->Position, desiredPos, 0.02f);

		glm::vec3 newFront = glm::normalize(center - m_Camera->Position);
        m_Camera->Front = glm::mix(m_Camera->Front, newFront, 0.05f);

        if (glm::length(m_Camera->Position - desiredPos) < 1.0f) {
            m_ReturningToCenter = false;
        }
    }
    else {
		m_Camera->Position = glm::vec3(x, y, z);
		m_Camera->Front = glm::normalize(center - m_Camera->Position);
    }

    m_Camera->updateCameraVectorsFront();

    // === Co jakiś czas losowe "lock-in" ===
    static float lockTimer = 0.0f;
    lockTimer += dt;
    if (lockTimer > 20.0f + rand() % 6) {
        if (!planets.empty()) {
            int idx = rand() % planets.size();
            LockOnPlanet(planets[idx].get(), 6.0f);
        }
        lockTimer = 0.0f;
    }
}

void CameraController::LockOnPlanet(Planet* planet, float duration)
{
    m_IsLocked = true;
    m_TargetPlanet = planet;
    m_LockOnDuration = duration;
}
