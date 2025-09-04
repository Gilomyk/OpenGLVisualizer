#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera {
private:
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

public:
	Camera(float fov, float aspectRatio, float nearPlane, float farPlane);

	void SetPosition(const glm::vec3& position);
	void LookAt(const glm::vec3& target);

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix() const;
};