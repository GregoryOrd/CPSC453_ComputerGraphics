#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "A_RecursiveShapeScene.h"

enum DIRECTION { LEFT, RIGHT };

class DragonCurve : public A_RecursiveShapeScene
{
public:
	DragonCurve(std::vector<Point>& originalLine, int minIterations, int maxIterations);

	int groupingSize() const override;
	Colour vertexColour(int vertex) override;

private:
	void findShapesRecursively(std::vector<Point>& line, int iterations) override;
	void pushShapesIntoVertices(std::vector<Point>& line) override;

	Point markPoint(float sideLength, Point startingPoint, float angleFromXAxis);
	void dragonCurveRecursion(int iteration, float sideLength, DIRECTION direction, Point startingPoint, float startingAngleFromXAxis);
};
