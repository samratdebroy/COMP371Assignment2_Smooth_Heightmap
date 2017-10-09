#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glm.hpp"

#include <string>
#include <glew.h>
#include <iostream>
#include  <vector>
#include <math.h> 
#include <algorithm>

using namespace std;

class Terrain
{
public:
	Terrain();
	void init(std::string heightmapPath);
	~Terrain();
	vector<float> getVertices(int width, int height);
	vector<int> getIndices(int width, int height);
	void Draw(GLenum renderMode);
	int getOriginalWidth() const;
	int getOriginalHeight() const;
	void setSkipSize(int skipSize);
	void nextState(float value);
private:
	int width;
	int height;
	int nrComponents;

	int originalWidth;
	int originalHeight;
	vector<float> originalVertices;

	// Store State
	enum STATE {NORMAL, REDUCED, CATMULLX, CATMULLZ};
	STATE state = NORMAL;

	vector<float> vertices ;
	vector<int> indices;
	int getVerticesCount(int width, int height);
	int getIndicesCount(int width, int height);
	unsigned char *heightMapData;

	void getCatMullXVertices(float stepSize);
	void getCatMullZVertices(double stepSize);

	/* Render Data */
	unsigned int VAO, VBO, EBO;
	void setupMesh(bool init);
};

