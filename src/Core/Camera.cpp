#include "Camera.h"

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
	: position(0.0f, 0.0f, 3.0f), target(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f)
{
	projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	viewMatrix = glm::lookAt(position, target, up);
}

void Camera::SetPosition(const glm::vec3& pos) {
	position = pos;
	viewMatrix = glm::lookAt(position, target, up);
}

glm::vec3 Camera::GetPosition() const {
	return position;
}

void Camera::LookAt(const glm::vec3& tgt) {
	target = tgt;
	viewMatrix = glm::lookAt(position, target, up);
}

glm::mat4 Camera::GetViewMatrix() const {
	return viewMatrix;
}

glm::mat4 Camera::GetProjectionMatrix() const {
	return projectionMatrix;
}