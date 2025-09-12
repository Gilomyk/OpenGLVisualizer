#pragma once
#include <glad/glad.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// inspirowanie siê: https://learnopengl.com/Getting-started/Camera
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UPWARD,
	DOWNWARD
};

class Camera {
public:
	// Parametry domyœlne
	float Yaw = -90.0f;
	float Pitch = 0.0f;
	float MovementSpeed = 2.5f;
	float MouseSensitivity = 0.1f;
	float Zoom = 45.0f;

	// Parametry ruchu z przyspieszeniem
	bool enableAcceleration = true;
	float acceleration = 500.0f;
	float maxSpeed = 1000.0f;
	float Damping = 5.0f;
	glm::vec3 m_Velocity = glm::vec3(0.0f);

	// Dane kamery
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	// Macierze
	glm::mat4 Projection;

	Camera(float fov, float aspectRatio, float nearPlane, float farPlane);
	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;

	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
	void ProcessMouseScroll(float yoffset);
	void Update(float deltaTime);

	glm::vec3 GetPosition() const { return Position; }

	void CenterOn(const glm::vec3& targetPos);
private:
	void updateCameraVectors();
};