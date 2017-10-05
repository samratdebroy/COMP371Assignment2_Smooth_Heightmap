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
	Terrain();
	void init(std::string heightmapPath);
	~Terrain();
	vector<float> getVertices(int width, int height);
	vector<int> getIndices(int width, int height);
	void Draw(GLenum renderMode);
	int getOriginalWidth() const;
	int getOriginalHeight() const;
	void setSkipSize(int skipSize);
private:
	int width;
	int height;
	vector<float> vertices ;
	int originalWidth;
	int originalHeight;
	vector<float> originalVertices;
	vector<int> indices;
	int getVerticesCount(int width, int height);
	int getIndicesCount(int width, int height);
	int nrComponents;
	unsigned char *heightMapData;
	/* Render Data */
	unsigned int VAO, VBO, EBO;
	void setupMesh(bool init);
};

