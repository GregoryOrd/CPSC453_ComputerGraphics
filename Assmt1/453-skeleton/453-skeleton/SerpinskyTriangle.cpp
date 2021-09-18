#include "SerpinskyTriangle.h"

SerpinskyTriangle::SerpinskyTriangle(TrianglePositions& originalTriangle, int minIterations, int maxIterations)
	: originalTriangle_(originalTriangle)
	, currentIterations_(minIterations)
	, minIterations_(minIterations)
	, maxIterations_(maxIterations)
{
	findTrianglesRecursively(originalTriangle, minIterations);
}

void SerpinskyTriangle::findTrianglesRecursively(TrianglePositions& triangle, int iterations)
{
	if (iterations == 0)
	{
		pushTriangleIntoSerpinksyVertices(triangle);
		return;
	}

	Point cornerA = triangle[0];
	Point cornerB = triangle[1];
	Point cornerC = triangle[2];

	Point cornerD = 0.5f * cornerA + 0.5f * cornerB;
	Point cornerE = 0.5f * cornerB + 0.5f * cornerC;
	Point cornerF = 0.5f * cornerC + 0.5f * cornerA;

	TrianglePositions newTriangleA = { cornerD, cornerB, cornerE };
	TrianglePositions newTriangleB = { cornerA, cornerD, cornerF };
	TrianglePositions newTriangleC = { cornerF, cornerE, cornerC };

	if (iterations > 1)
	{
		findTrianglesRecursively(newTriangleA, iterations - 1);
		findTrianglesRecursively(newTriangleB, iterations - 1);
		findTrianglesRecursively(newTriangleC, iterations - 1);
	}
	else
	{
		pushTriangleIntoSerpinksyVertices(newTriangleA);
		pushTriangleIntoSerpinksyVertices(newTriangleB);
		pushTriangleIntoSerpinksyVertices(newTriangleC);
	}
}

void SerpinskyTriangle::pushTriangleIntoSerpinksyVertices(TrianglePositions& triangle)
{
	for (int i = 0; i < 3; i++)
	{
		serpinksyVertices_.push_back(triangle[i]);
	}
}

size_t SerpinskyTriangle::numVertices()
{
	return serpinksyVertices_.size();
}

std::vector<Point> SerpinskyTriangle::serpinksyVertices()
{
	return serpinksyVertices_;
}

bool SerpinskyTriangle::incrementIterations()
{
	if (currentIterations_ < maxIterations_)
	{
		currentIterations_++;
		serpinksyVertices_.clear();
		findTrianglesRecursively(originalTriangle_, currentIterations_);
		return true;
	}
	return false;
}

bool SerpinskyTriangle::decrementIterations()
{
	if (currentIterations_ > minIterations_)
	{
		currentIterations_--;
		serpinksyVertices_.clear();
		findTrianglesRecursively(originalTriangle_, currentIterations_);
		return true;
	}
	return false;
}
