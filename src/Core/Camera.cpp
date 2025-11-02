#include "Camera.h"
#include <iostream>

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
	: Front(glm::vec3(0.0f, 0.0f, -1.0f)),
	MovementSpeed(1000.0f),
	MouseSensitivity(0.1f),
	Zoom(fov),
	Position(glm::vec3(0.0f, 100.0f, 600.0f)),
	WorldUp(glm::vec3(0.0f, 1.0f, 0.0f))
{
	Projection = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
	return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
	return Projection;
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
	glm::vec3 accel(0.0f);

	float currentAcc = acceleration;

	if (direction == FORWARD)  accel += Front * currentAcc;
	if (direction == BACKWARD) accel -= Front * currentAcc;
	if (direction == LEFT)     accel -= Right * currentAcc;
	if (direction == RIGHT)    accel += Right * currentAcc;
	if (direction == UPWARD)   accel += Up * currentAcc;
	if (direction == DOWNWARD) accel -= Up * currentAcc;

	m_Velocity += accel * deltaTime;

	// Clamp
	if (glm::length(m_Velocity) > maxSpeed) {
		m_Velocity = glm::normalize(m_Velocity) * maxSpeed;
	}
}

void Camera::Update(float deltaTime) {
	// Ruch kamery wg Velocity
	Position += m_Velocity * deltaTime;

	// Hamowanie naturalne
	m_Velocity -= m_Velocity * Damping * deltaTime;

	// Zapobiegamy "oscylacjom" – jak prêdkoœæ bardzo ma³a to zerujemy
	if (glm::length(m_Velocity) < 0.001f)
		m_Velocity = glm::vec3(0.0f);
}



void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
	xoffset *= MouseSensitivity;
	yoffset *= MouseSensitivity;

	Yaw += xoffset;
	Pitch += yoffset;

	if (constrainPitch) {
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}
	updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
	Zoom -= (float)yoffset;
	if (Zoom < 1.0f)
		Zoom = 1.0f;
	if (Zoom > 45.0f)
		Zoom = 45.0f;
	Projection = glm::perspective(glm::radians(Zoom), Projection[1][1] / Projection[0][0], 0.1f, 1000.0f);
}

void Camera::CenterOn(const glm::vec3& targetPos) {

	// Obliczamy front wzglêdem nowego celu
	Front = glm::normalize(targetPos - Position);

	// Od fronta i worldUp liczymy Right i Up
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));

	Yaw = -90.0f;
	Pitch = 0.0f;
	Position = glm::vec3(0.0f, 0.0f, 300.0f);
}


void Camera::updateCameraVectors() {
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::updateCameraVectorsFront() {
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));
}