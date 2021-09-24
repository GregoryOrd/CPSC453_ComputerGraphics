#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "A_RecursiveShapeScene.h"

static const Colour lineColours[4] = {
	Colour(1.f, 0.f, 0.f), // Flat Left    - red
	Colour(0.f, 1.f, 0.f), // Angled Left  - green
	Colour(0.f, 1.f, 0.f),  // Angled Right - green
	Colour(1.f, 0.f, 0.f)  // Flat Right   - red
};

class KochSnowflake : public A_RecursiveShapeScene
{
public:
	KochSnowflake(std::vector<Point>& originalLine, int minIterations, int maxIterations);

	int groupingSize() const override;
	Colour vertexColour(int vertex) override;

private:
	void findShapesRecursively(std::vector<Point>& line, int iterations) override;
	void pushShapesIntoVertices(std::vector<Point>& line) override;
};
