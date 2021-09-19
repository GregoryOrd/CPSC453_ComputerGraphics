#include "SerpinskyTriangle.h"

SerpinskyTriangle::SerpinskyTriangle(std::vector<Point>& originalTriangle, int minIterations, int maxIterations)
	: A_RecursiveShapeScene(originalTriangle, minIterations, maxIterations)
{
	findShapesRecursively(originalTriangle, minIterations);
}

void SerpinskyTriangle::findShapesRecursively(std::vector<Point>& triangle, int iterations)
{
	if (iterations == 0)
	{
		pushShapesIntoVertices(triangle);
		return;
	}

	Point cornerA = triangle[0];
	Point cornerB = triangle[1];
	Point cornerC = triangle[2];

	Point cornerD = 0.5f * cornerA + 0.5f * cornerB;
	Point cornerE = 0.5f * cornerB + 0.5f * cornerC;
	Point cornerF = 0.5f * cornerC + 0.5f * cornerA;

	std::vector<Point> newTriangleA = { cornerD, cornerB, cornerE };
	std::vector<Point> newTriangleB = { cornerA, cornerD, cornerF };
	std::vector<Point> newTriangleC = { cornerF, cornerE, cornerC };

	if (iterations > 1)
	{
		findShapesRecursively(newTriangleA, iterations - 1);
		findShapesRecursively(newTriangleB, iterations - 1);
		findShapesRecursively(newTriangleC, iterations - 1);
	}
	else
	{
		pushShapesIntoVertices(newTriangleA);
		pushShapesIntoVertices(newTriangleB);
		pushShapesIntoVertices(newTriangleC);
	}
}

void SerpinskyTriangle::pushShapesIntoVertices(std::vector<Point>& triangle)
{
	for (int i = 0; i < 3; i++)
	{
		vertices_.push_back(triangle[i]);
	}
}
