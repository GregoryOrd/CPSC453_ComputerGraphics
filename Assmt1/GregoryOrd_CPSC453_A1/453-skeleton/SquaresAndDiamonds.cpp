#include "SquaresAndDiamonds.h"
#include <math.h>

SquaresAndDiamonds::SquaresAndDiamonds(std::vector<Point>& originalSquare, int minIterations, int maxIterations)
	: A_RecursiveShapeScene(originalSquare, minIterations, maxIterations)
{
	findShapesRecursively(originalSquare, minIterations);
}

void SquaresAndDiamonds::findShapesRecursively(std::vector<Point>& square, int iterations)
{
	pushShapesIntoVertices(square);
	if (iterations == 0)
	{
		return;
	}

	Point cornerA = square[0];
	Point cornerB = square[1];
	Point cornerC = square[2];
	Point cornerD = square[3];

	Point cornerE = 0.5f * cornerA + 0.5f * cornerB;
	Point cornerF = 0.5f * cornerB + 0.5f * cornerC;
	Point cornerG = 0.5f * cornerC + 0.5f * cornerD;
	Point cornerH = 0.5f * cornerD + 0.5f * cornerA;

	std::vector<Point> newSquare = { cornerE, cornerF, cornerG, cornerH };

	if (iterations > 1)
	{
		findShapesRecursively(newSquare, iterations - 1);
	}
	else
	{
		pushShapesIntoVertices(newSquare);
	}
}

void SquaresAndDiamonds::pushShapesIntoVertices(std::vector<Point>& square)
{
	for (int i = 0; i < 4; i++)
	{
		vertices_.push_back(square[i]);
	}
	//vertices_.push_back(square[0]);
}

int SquaresAndDiamonds::groupingSize() const
{
	//Draw 4 vertices at a time
	return 4;
}

bool SquaresAndDiamonds::isDiagonal(int vertex)
{
	return (vertex / 4) % 2;
}

Colour SquaresAndDiamonds::vertexColour(int vertex)
{
	float iterationColourIncrement = 0.07f * (vertex / 4);

	Colour modifiedSquareColour = {
		layerColours[0][0] - iterationColourIncrement,
		layerColours[0][1] + iterationColourIncrement,
		layerColours[0][2] + iterationColourIncrement,
	};
	Colour modifiedDiagonalColour = {
		layerColours[1][0] + iterationColourIncrement,
		layerColours[1][1] - iterationColourIncrement,
		layerColours[1][2] + iterationColourIncrement,
	};


	if (isDiagonal(vertex))
	{
		return modifiedDiagonalColour;
	}
	else
	{
		return modifiedSquareColour;
	}
}
