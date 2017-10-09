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
	state = REDUCED;

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
		tempVertices.reserve(getVerticesCount(newWidth, newHeight));

		// Populate Vertex positions
		for (int row = 0; row < newHeight; row++)
		{
			for (int col = 0; col <newWidth; col++)
			{
				int index = (row*skipSize*originalWidth + col*skipSize) * 3;
				tempVertices.push_back( originalVertices[index + 0] );	// x pos
				tempVertices.push_back(originalVertices[index + 1] );	// y pos
				tempVertices.push_back(originalVertices[index + 2] );	// z pos
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

void Terrain::nextState(float value)
{
	if(state == REDUCED)
	{
		getCatMullXVertices(value);
		state = CATMULLX;
	}
	else if (state == CATMULLX)
	{
		getCatMullZVertices(value);
		state = CATMULLZ;
	}
}

int Terrain::getVerticesCount(int width, int height)
{
	return width * height * 3;
}

int Terrain::getIndicesCount(int width, int height)
{
	int numTriStrips = height - 1; // number of triangle strips required
	int numDegenIndices = 2 * (numTriStrips - 1); // Number of degenerate indices required (used to move from one strip to another)
	int verticesPerStrip = 2 * width;
	return (verticesPerStrip * numTriStrips + numDegenIndices);
}

void Terrain::getCatMullXVertices(float stepSize)
{
	// store width and height of reduced vertices
	int numPtsPerSegment = ceil( 1.0f/ stepSize); // assuming 0.0 < stepSize < 1.0 and excluding one end point
	int newWidth = numPtsPerSegment * (width-1) + 1; // numPtsPerSegment * numSegmentsPerRow + 1 end point
	int newHeight = height;

	vector<float> tempVertices;
	tempVertices.reserve(getVerticesCount(newWidth, newHeight));

	glm::vec3 p0, p1, p2, p3;

	// Populate Vertex positions
	for (int row = 0; row < height; row++)
	{
		// Get first two points of the row
		int i = row*width * 3;
		p0 = glm::vec3(vertices[i+ 0], vertices[i + 1], vertices[i + 2]);
		p1 = glm::vec3(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
		// Linear interpolation between first Point and 2nd point
		for (float u = 0.0f; u < 1.0f; u += stepSize)
		{
			glm::vec3 point = p0 + u *(p1 - p0);
			// Add point as a vertex for new mesh
			tempVertices.push_back(point.x);
			tempVertices.push_back(point.y);
			tempVertices.push_back(point.z);
		}

		// Do CatMull Rom on every point
		for (int col = 1; col < width - 2; col++)
		{
			// Get four points
			int index = (row*width + col) * 3;
			p0 = glm::vec3(vertices[index - 3], vertices[index - 2], vertices[index - 1]);
			p1 = glm::vec3(vertices[index + 0], vertices[index + 1], vertices[index + 2]);
			p2 = glm::vec3(vertices[index + 3], vertices[index + 4], vertices[index + 5]);
			p3 = glm::vec3(vertices[index + 6], vertices[index + 7], vertices[index + 8]);

			// Evaluate and push CatMullPoints
			glm::vec3 point;
			for(float u = 0.0f; u < 1.0f; u += stepSize)
			{			
				// Calculate the next interpolated curve point
				point = 0.5f * (
					(2.0f * p1) +
					(-p0 + p2)*u +
					(2.0f * p0 - 5.0f * p1 + 4.0f *p2 - p3)*u*u +
					(-p0 + 3.0f*p1 - 3.0f*p2 + p3)*u*u*u);

				// Add point as a vertex for new mesh
				tempVertices.push_back(point.x);
				tempVertices.push_back(point.y);
				tempVertices.push_back(point.z);
			}
		}

		// Get last two points of the row
		i = (row*width + (width - 2)) * 3;
		p0 = glm::vec3(vertices[i + 0], vertices[i + 1], vertices[i+ 2]);
		p1 = glm::vec3(vertices[i + 3], vertices[i + 4], vertices[i+ 5]);
		// Linear interpolation between 2nd to last Point and last point
		for (float u = 0.0f; u < 1.0f; u += stepSize)
		{
			glm::vec3 point = p0 + u *(p1 - p0);
			// Add point as a vertex for new mesh
			tempVertices.push_back(point.x);
			tempVertices.push_back(point.y);
			tempVertices.push_back(point.z);
		}

		// Add last point in row
		// Add point as a vertex for new mesh
		tempVertices.push_back(p1.x);
		tempVertices.push_back(p1.y);
		tempVertices.push_back(p1.z);
	}

	// Overwrite the global values of the Terrain
	width = newWidth;
	height = newHeight;
	vertices = tempVertices;

	// Reset the Mesh
	indices.clear();
	getIndices(width, height);
	setupMesh(false);
}

void Terrain::getCatMullZVertices(float stepSize)
{

	// store width and height of reduced vertices
	int numPtsPerSegment = ceil(1.0f / stepSize); // assuming 0.0 < stepSize < 1.0 and excluding one end point
	int newHeight = numPtsPerSegment * (height - 1) + 1; // numPtsPerSegment * numSegmentsPerCol + 1 end point
	int newWidth = width;

	vector<float> tempVertices;
	tempVertices.resize(getVerticesCount(newWidth, newHeight));

	glm::vec3 p0, p1, p2, p3;

	// Populate Vertex positions
	for (int col  = 0; col < width; col++)
	{
		// Get first two points of the col
		int i = col * 3;
		p0 = glm::vec3(vertices[i + 0], vertices[i + 1], vertices[i + 2]);
		p1 = glm::vec3(vertices[width*3 + i + 0], vertices[width*3 + i + 1], vertices[width*3 + i + 2]);
		// Linear interpolation between first Point and 2nd point
		int newRow = 0;
		for (float u = 0.0f; u < 1.0f; u += stepSize)
		{
			glm::vec3 point = p0 + u *(p1 - p0);
			int index = (newRow*newWidth + col )* 3;
			// Add point as a vertex for new mesh
			tempVertices[index + 0] = point.x;
			tempVertices[index + 1] = point.y;
			tempVertices[index + 2] = point.z;
			newRow++;
		}

		// Do CatMull Rom on every point
		for (int row = 1; row < height - 2; row++)
		{
			// Get four points
			auto index = [&](int offset) {return (width*(row+ offset) + col) * 3; };
			p0 = glm::vec3(vertices[index(-1) + 0], vertices[index(-1) + 1], vertices[index(-1) +2]);
			p1 = glm::vec3(vertices[index(0) + 0], vertices[index(0) + 1], vertices[index(0) +2]);
			p2 = glm::vec3(vertices[index(1) + 0], vertices[index(1) + 1], vertices[index(1) +2]);
			p3 = glm::vec3(vertices[index(2) + 0], vertices[index(2) + 1], vertices[index(2) +2]);

			// Evaluate and push CatMullPoints
			float u = 0.0f; // Current step
			glm::vec3 point;
			for (float u = 0.0f; u < 1.0f; u += stepSize)
			{
				// Calculate the next interpolated curve point
				point = 0.5f * (
					(2.0f * p1) +
					(-p0 + p2)*u +
					(2.0f * p0 - 5.0f * p1 + 4.0f *p2 - p3)*u*u +
					(-p0 + 3.0f*p1 - 3.0f*p2 + p3)*u*u*u);

				// Add point as a vertex for new mesh
				int i = (newRow*newWidth + col) * 3;
				tempVertices[i + 0] = point.x;
				tempVertices[i + 1] = point.y;
				tempVertices[i + 2] = point.z;
				newRow++;
			}
		}

		// Get last two points of the col
		i = (width*(height-2) + col) * 3;
		p0 = glm::vec3(vertices[i + 0], vertices[i + 1], vertices[i + 2]);
		p1 = glm::vec3(vertices[width * 3 + i + 0], vertices[width * 3 + i + 1], vertices[width * 3 + i + 2]);
		// Linear interpolation between first Point and 2nd point
		for (float u = 0.0f; u < 1.0f; u += stepSize)
		{
			glm::vec3 point = p0 + u *(p1 - p0);
			int index = (newRow*newWidth + col) * 3;
			// Add point as a vertex for new mesh
			tempVertices[index + 0] = point.x;
			tempVertices[index + 1] = point.y;
			tempVertices[index + 2] = point.z;
			newRow++;
		}

		// Add last point in column
		tempVertices[(newWidth*newRow + col)*3+ 0] = p1.x;
		tempVertices[(newWidth*newRow + col) * 3 + 1] = p1.y;
		tempVertices[(newWidth*newRow + col) * 3 + 2] = p1.z;
	}

	// Overwrite the global values of the Terrain
	width = newWidth;
	height = newHeight;
	vertices = tempVertices;

	// Reset the Mesh
	indices.clear();
	getIndices(width, height);
	setupMesh(false);
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
