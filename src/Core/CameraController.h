#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Camera.h"
#include "../Scene/Planet.h"
#include <vector>

class CameraController {
public:
    CameraController(Camera* camera)
        : m_Camera(camera), m_OrbitRadius(800.0f), m_OrbitSpeed(0.2f),
        m_OrbitAngle(0.0f), m_LockOnDuration(0.0f), m_TempoFactor(1.0f),
        m_IsLocked(false), m_TargetPlanet(nullptr)
    {
    }

    void Update(float dt, const std::vector<std::unique_ptr<Planet>>& planets, float localTempo);
    void LockOnPlanet(Planet* planet, float duration);

    void SetOrbitParams(float radius, float speed) {
        m_OrbitRadius = radius;
        m_OrbitSpeed = speed;
    }

private:
    Camera* m_Camera;
    float m_OrbitRadius;
    float m_OrbitSpeed;
    float m_OrbitAngle;
    float m_LockOnDuration;
    float m_TempoFactor;
    bool m_IsLocked;
	bool m_ReturningToCenter = false;

    Planet* m_TargetPlanet;
};
