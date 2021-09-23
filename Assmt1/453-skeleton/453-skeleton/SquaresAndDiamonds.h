#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "A_RecursiveShapeScene.h"

static const Colour layerColours[3] = {
	Colour(1.f, 0.f, 0.f), // Squares
	Colour(0.f, 1.f, 0.f), // Diamonds  - green
};

class SquaresAndDiamonds : public A_RecursiveShapeScene
{
public:
	SquaresAndDiamonds(std::vector<Point>& originalSquare, int minIterations, int maxIterations);

	int groupingSize() const override;
	Colour vertexColour(int vertexNumber) override;

private:
	bool isDiagonal(int vertex);
	void findShapesRecursively(std::vector<Point>& square, int iterations) override;
	void pushShapesIntoVertices(std::vector<Point>& square) override;
};
