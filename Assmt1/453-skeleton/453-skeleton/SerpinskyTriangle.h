#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "A_RecursiveShapeScene.h"

static const Colour triangleColours[3] = {
	Colour(1.f, 0.f, 0.f), // top triangle          - red
	Colour(0.f, 1.f, 0.f), // bottom left triangle  - green
	Colour(0.f, 0.f, 1.f)  // bottom right triangle - blue
};

class SerpinskyTriangle : public A_RecursiveShapeScene
{
public:
	SerpinskyTriangle(std::vector<Point>& originalTriangle, int minIterations, int maxIterations);

	int groupingSize() const override;
	Colour vertexColour(int vertex) override;

private:
	void findShapesRecursively(std::vector<Point>& triangle, int iterations) override;
	void pushShapesIntoVertices(std::vector<Point>& triangle) override;
};
