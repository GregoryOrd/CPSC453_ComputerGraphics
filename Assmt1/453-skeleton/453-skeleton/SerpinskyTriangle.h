#pragma once

#include <vector>
#include <glm/glm.hpp>

typedef glm::vec3 Point;
typedef Point TrianglePositions[3];

class SerpinskyTriangle
{
public:
	SerpinskyTriangle(TrianglePositions& originalTriangle, int minIterations, int maxIterations);

	size_t numVertices();
	std::vector<Point> serpinksyVertices();
	bool incrementIterations();
	bool decrementIterations();

private:
	void findTrianglesRecursively(TrianglePositions& triangle, int iterations);
	void pushTriangleIntoSerpinksyVertices(TrianglePositions& triangle);

private:
	int currentIterations_;
	int minIterations_;
	int maxIterations_;
	std::vector<Point> serpinksyVertices_;
	TrianglePositions& originalTriangle_;
};
