#include "Camera.h"
#include <matrix_transform.inl>

Camera::Camera(glm::vec3 Position)
{
	position = Position;
	yaw = 0;
	pitch = 0;
	updateCameraOrientation();
}

void Camera::updateCameraOrientation()
{
	// Get local axes from the orientation matrix
	forward = glm::vec3(glm::inverse(orientation()) * glm::vec4(0, 0, -1, 1)); // in OpenGL -z points into the screen
	right = glm::vec3(glm::inverse(orientation()) * glm::vec4(1, 0, 0, 1));
	up = glm::vec3(glm::inverse(orientation()) * glm::vec4(0, 1, 0, 1));

	// Normalize all the axes direction vectors
	forward = glm::normalize(forward);
	right = glm::normalize(right);
	up = glm::normalize(up);
}

/**
 * returns rotation matrix representing the camera's orientation depending on current pitch/yaw angles
 */
glm::mat4 Camera::orientation()
{
	glm::mat4 rotationMat(1.0f);
	rotationMat = glm::rotate(rotationMat,glm::radians(pitch), glm::vec3(1, 0, 0)); // vertical angle around x-axis
	rotationMat = glm::rotate(rotationMat, glm::radians(yaw), glm::vec3(0, 1, 0)); // horizontal angle around y-axis
	return rotationMat;
}

glm::mat4 Camera::ViewMatrix()
{
	return glm::lookAt(position, position + forward, up);
}

void Camera::DisplacePosition(glm::vec3 displacement)
{
	position += displacement;
}

void Camera::ChangePitch(float offset)
{
	pitch += offset;
	updateCameraOrientation();
}

void Camera::ChangeYaw(float offset)
{
	yaw += offset;
	updateCameraOrientation();
}

glm::vec3 Camera::forwardDirection() const
{
	return forward;
}

glm::vec3 Camera::rightDirection() const
{
	return right;
}

glm::vec3 Camera::upDirection() const
{
	return up;
}
