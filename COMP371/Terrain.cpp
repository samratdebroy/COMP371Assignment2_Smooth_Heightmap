#include "Terrain.h"

void Terrain::init(std::string heightmapPath)
{

	heightMapData = stbi_load(heightmapPath.c_str(), &width, &height, &nrComponents, 0);

	if (!heightMapData)
	{
		std::cout << "Texture failed to load at path: " << heightmapPath << std::endl;
		stbi_image_free(heightMapData);
	}

	getVertices(width, height);
	getIndices(width, height);
	setupMesh(true);
}


Terrain::Terrain()
{
}

Terrain::~Terrain()
{
	stbi_image_free(heightMapData);
}

vector<float> Terrain::getVertices(int width, int height)
{
	if (!vertices.empty()) return vertices;

	vertices.resize(getVerticesCount(width,height)) ;

	int i = 0;

	// Populate Vertex positions
	for (int row = 0; row < height; row++)
	{
		for (int col = 0; col <width; col++)
		{
			vertices[i++] = (float)col;	// x pos
			vertices[i++] = heightMapData[(row*width + col)*nrComponents]/255.0f; // y pos from first color value of pixel
			vertices[i++] = (float)row;	// z pos
		}
	}

	originalVertices = vertices;
	originalWidth = width;
	originalHeight = height;

	return vertices;
}

vector<int> Terrain::getIndices(int width, int height)
{
	if (!indices.empty()) return indices;

	indices.resize(getIndicesCount(width, height));

	int numTriStrips = height - 1; // number of triangle strips required
	int offset = 0;

	for (int y = 0; y < numTriStrips; y++)
	{
		// Repeat the first vertex to complete the degenerate triangles from the last Tri strip
		if (y > 0)
			indices[offset++] = y * width; // first vertex of new strip

										   // Add the indices of the vertices on the triangle strip
		for (int x = 0; x < width; x++)
		{
			indices[offset++] = y * width + x; // Top row of the triangle strip
			indices[offset++] = (y + 1) * width + x; // bottom row of the triangle strip
		}

		// Repeat the last vertec for the degenerate triangle to the next triangle strip
		if (y < height - 2)
			indices[offset++] = (y + 2) * width - 1; // last vertex of curr strip
	}
	return indices;
}

void Terrain::Draw(GLenum renderMode)
{
	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(renderMode, getIndicesCount(width, height), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

int Terrain::getOriginalWidth() const
{
	return originalWidth;
}

int Terrain::getOriginalHeight() const
{
	return originalHeight;
}

void Terrain::setSkipSize(int skipSize)
{
	if(skipSize == 1)
	{
		// Overwrite the global values of the Terrain
		width = originalWidth;
		height = originalHeight;
		vertices = originalVertices;
	}else
	{
		// Resize the width and height to adjust for the skip size
		int newWidth = originalWidth / skipSize;
		int newHeight = originalHeight / skipSize;

		vector<float> tempVertices;
		tempVertices.resize(getVerticesCount(newWidth, newHeight));

		int i = 0;

		// Populate Vertex positions
		for (int row = 0; row < newHeight; row++)
		{
			for (int col = 0; col <newWidth; col++)
			{
				int index = (row*skipSize*originalWidth + col*skipSize) * 3;
				tempVertices[i++] = originalVertices[index + 0];	// x pos
				tempVertices[i++] = originalVertices[index + 1];	// y pos
				tempVertices[i++] = originalVertices[index + 2];	// z pos
			}
		}

		// Overwrite the global values of the Terrain
		width = newWidth;
		height = newHeight;
		vertices = tempVertices;
	}


	// Reset the Mesh
	indices.clear();
	getIndices(width, height);
	setupMesh(false);

}

int Terrain::getVerticesCount(int width, int height)
{
	return width * height * 3; //TODO maybe add normals in the future
}

int Terrain::getIndicesCount(int width, int height)
{
	int numTriStrips = height - 1; // number of triangle strips required
	int numDegenIndices = 2 * (numTriStrips - 1); // Number of degenerate indices required (used to move from one strip to another)
	int verticesPerStrip = 2 * width;
	return (verticesPerStrip * numTriStrips + numDegenIndices);
}

void Terrain::setupMesh(bool init = true)
{
	if(init)
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
	}

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, getVerticesCount(width, height) * sizeof(float), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, getIndicesCount(width, height) * sizeof(int), &indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

	glBindVertexArray(0);
}
