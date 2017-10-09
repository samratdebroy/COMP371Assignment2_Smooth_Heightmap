
#include "stdafx.h"
#include "..\glew\glew.h"	// include GL Extension Wrangler
#include "..\glfw\glfw3.h"	// include GLFW helper library
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

#include "Camera.h"
#include "Shader.h"
#include "Terrain.h"

using namespace std;

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 800;

glm::vec3 camera_position;
glm::vec3 triangle_scale;
glm::mat4 projection;

// Camera Settings
Camera camera(glm::vec3(0.0f,2.0f, 3.0f)); // Camera with starting position
float lastX = WIDTH / 2.0f, lastY = HEIGHT / 2.0f; // start at center of screen
bool firstMouse = true; // flag for first mouse movement

// Mouse Buttons held down
bool leftButtonClicked = false;

// Timing Variables
float deltaTime = 0.0f; // Time b/w last frame and current frame
float lastFrame = 0.0f;
float lastSkipSizeUpdate = 0.0f;

// Player controlled variables
GLenum drawMode = GL_TRIANGLE_STRIP;

// Prototype
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);
void setDrawMode(GLenum newDrawMode);
void reset();

// Terrain with HeightMap
Terrain terrain;
Terrain origTerrain;
bool showOriginalTerrain = false;
float stepSize;

// The MAIN function, from here we start the application and run the game loop
int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Load one cube", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Set the callback functions for mouse movements and frame size change
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	// Define the viewport dimensions
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST); // enable the z-buffer and depth testing
	glDepthFunc(GL_LESS); // re-enable the depth buffer to normal


	projection = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.01f, 100.0f);

	triangle_scale = glm::vec3(0.01f);

	// Terrain Plain
	terrain.init("heightmaps/depth.bmp");
	origTerrain.init("heightmaps/depth.bmp");
	Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");

	// Ask user for skipSize and stepSize for CatMull operations
	reset();

		// Game loop
		while (!glfwWindowShouldClose(window))
		{
			// per-frame Time logic
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			// Handle inputs
			processInput(window);

			// Render
			// Clear the colorbuffer
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// View matrix
			glm::mat4 view;
			view = camera.ViewMatrix();

			// Terrain
			terrainShader.UseProgram();
			glm::mat4 model(1.0f);
			model = glm::scale(model, triangle_scale);
			model = glm::translate(model, glm::vec3(-terrain.getOriginalWidth() / 2.0f, -0.75f, -terrain.getOriginalHeight() / 2.0f));
			terrainShader.setMat4("projection", projection);
			terrainShader.setMat4("view", view);
			terrainShader.setMat4("model", model);
			if(showOriginalTerrain)
			{
				origTerrain.Draw(drawMode);
			}else
			{
				terrain.Draw(drawMode);
			}

			// Swap the screen buffers
			glfwSwapBuffers(window);
			// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
			glfwPollEvents();

		}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
	// Exits the application if escape was pressed
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// Camera Position keys
	float cameraSensitivity = 15.0f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		camera.DisplacePosition(camera.forwardDirection() * cameraSensitivity);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		camera.DisplacePosition(-camera.forwardDirection() * cameraSensitivity);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		camera.DisplacePosition(camera.rightDirection() * cameraSensitivity);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		camera.DisplacePosition(-camera.rightDirection() * cameraSensitivity);


	// Scale up and Down
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		triangle_scale += 0.1 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		triangle_scale -= 0.1 * deltaTime;

	// CatMull
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS && glfwGetTime() - lastSkipSizeUpdate > 1)
	{
		terrain.nextState(stepSize);
		lastSkipSizeUpdate = glfwGetTime();
	}


	// Show original terrain buffer
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && glfwGetTime() - lastSkipSizeUpdate > 1)
	{
		showOriginalTerrain = !showOriginalTerrain;
		lastSkipSizeUpdate = glfwGetTime();
	}

	// Change Render Mode
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		setDrawMode(GL_TRIANGLE_STRIP);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		setDrawMode(GL_POINTS);

	if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS)
		reset();
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// left mouse button
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		leftButtonClicked = true;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		leftButtonClicked = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	// Calculate offsets
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top

	lastX = xpos;
	lastY = ypos;

	float cameraSensitivity = 0.1f;

	if(leftButtonClicked)
	{
		camera.ChangeYaw(xoffset * cameraSensitivity);
		camera.ChangePitch(yoffset * cameraSensitivity);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions
	glViewport(0, 0, width, height);
	// Update the project matrix to ensure we keep the proper aspect ratio
	projection = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.01f, 100.0f); 
}

void setDrawMode(GLenum newDrawMode)
{
	drawMode = newDrawMode;
}

void reset()
{
	int skipSize = 0;
	stepSize = 0;

	while(skipSize < 1 || skipSize > 300)
	{
		cout << "Enter reduce skip size between 1 to 300: ";
		cin >> skipSize;
	}

	while (stepSize < 0.01f || stepSize > 1.0f)
	{
		cout << "Enter CatMull step size between 0.01 to 1.0: ";
		cin >> stepSize;
	}

	terrain.setSkipSize(skipSize);

	camera.reset();
}