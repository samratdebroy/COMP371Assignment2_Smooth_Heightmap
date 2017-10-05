#pragma once
#include "..\glew\glew.h"
#include "glm.hpp"

class Camera
{
private:
	// Euler Angles
	float yaw, pitch;
	// Camera Position and Orientation
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	void updateCameraOrientation();
	glm::mat4 orientation();

public:
	Camera(glm::vec3 Position);
	glm::mat4 ViewMatrix();
	void DisplacePosition(glm::vec3 displacement);
	void ChangePitch(float offset);
	void ChangeYaw(float offset);

	glm::vec3 forwardDirection() const;
	glm::vec3 rightDirection() const;
	glm::vec3 upDirection() const;
};

