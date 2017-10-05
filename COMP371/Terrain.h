#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <string>
#include <glew.h>
#include <iostream>
#include  <vector>

using namespace std;

class Terrain
{
public:
	Terrain(std::string heightmapPath);
	~Terrain();
	vector<float> getVertices(int width, int height);
	vector<int> getIndices(int width, int height);
	void Draw(GLenum renderMode);
	int getWidth() const;
	int getHeight() const;

private:
	int width;
	int height;
	vector<float> vertices ;
	vector<int> indices;
	int getVerticesCount(int width, int height);
	int getIndicesCount(int width, int height);
	int nrComponents;
	unsigned char *heightMapData;
	/* Render Data */
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};

