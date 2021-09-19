#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "A_RecursiveShapeScene.h"

typedef glm::vec3 Point;

class SerpinskyTriangle : public A_RecursiveShapeScene
{
public:
	SerpinskyTriangle(std::vector<Point>& originalTriangle, int minIterations, int maxIterations);

private:
	void findShapesRecursively(std::vector<Point>& triangle, int iterations) override;
	void pushShapesIntoVertices(std::vector<Point>& triangle) override;
};
