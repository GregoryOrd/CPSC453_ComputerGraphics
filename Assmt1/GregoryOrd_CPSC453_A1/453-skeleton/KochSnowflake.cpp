#include "KochSnowflake.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

KochSnowflake::KochSnowflake(std::vector<Point>& originalTriangle, int minIterations, int maxIterations)
	: A_RecursiveShapeScene(originalTriangle, minIterations, maxIterations)
{
	findShapesRecursively(originalTriangle, minIterations);
}

void KochSnowflake::recurseOverLine(std::vector<Point>& line, int iterations)
{
	Point pointA = line[0];
	Point pointB = line[1];
	Point pointC = (2.f / 3.f) * pointA + (1.f / 3.f) * pointB;
	Point pointD = (1.f / 3.f) * pointA + (2.f / 3.f) * pointB;
	Point pointE = { 0.f, 0.f, 0.f };

	Point vectorAB = pointB - pointA;
	Point iDirectionVector = { 1.f, 0.f, 0.f };

	float lengthOfVectorAB = sqrt(vectorAB[0] * vectorAB[0] + vectorAB[1] * vectorAB[1] + vectorAB[2] * vectorAB[2]);
	float angleFromIDriectionVectorToABVector = acos(vectorAB[0] / lengthOfVectorAB);

	float newSideLength = (1.0f / 3.0f) * lengthOfVectorAB;
	if (vectorAB[1] >= 0)
	{
		pointE =
		{
			pointC[0] + newSideLength * cos(angleFromIDriectionVectorToABVector + (M_PI / 3)) ,
			pointC[1] + newSideLength * sin(angleFromIDriectionVectorToABVector + (M_PI / 3)),
			0.f
		};
	}
	else if (vectorAB[1] < 0)
	{
		pointE =
		{
			pointC[0] + newSideLength * cos((M_PI / 3) - angleFromIDriectionVectorToABVector) ,
			pointC[1] + newSideLength * sin((M_PI / 3) - angleFromIDriectionVectorToABVector),
			0.f
		};
	}

	std::vector<Point> flatLeft = { pointA, pointC };
	std::vector<Point> angledLeft = { pointC, pointE };
	std::vector<Point> angledRight = { pointE, pointD };
	std::vector<Point> flatRight = { pointD, pointB };

	if (iterations > 0)
	{
		recurseOverLine(flatLeft, iterations - 1);
		recurseOverLine(angledLeft, iterations - 1);
		recurseOverLine(angledRight, iterations - 1);
		recurseOverLine(flatRight, iterations - 1);
	}
	else
	{
		pushShapesIntoVertices(flatLeft);
		pushShapesIntoVertices(angledLeft);
		pushShapesIntoVertices(angledRight);
		pushShapesIntoVertices(flatRight);
	}
}

//In the case of the KochSnowflake we start with a triangle, but need to recurse over
//lines rather than recursing over triangles. Split the triangle into its legs,
//then call the recurse over line function.
void KochSnowflake::findShapesRecursively(std::vector<Point>& triangle, int iterations)
{
	std::vector<Point> lineA = { triangle[1], triangle[0] };
	std::vector<Point> lineB = { triangle[2], triangle[1] };
	std::vector<Point> lineC = { triangle[0], triangle[2] };

	if (iterations == 0)
	{
		pushShapesIntoVertices(lineA);
		pushShapesIntoVertices(lineB);
		pushShapesIntoVertices(lineC);
		return;
	}
	else
	{
		recurseOverLine(lineA, iterations - 1);
		recurseOverLine(lineB, iterations - 1);
		recurseOverLine(lineC, iterations - 1);
	}
}

void KochSnowflake::pushShapesIntoVertices(std::vector<Point>& line)
{
	for (int i = 0; i < 2; i++)
	{
		vertices_.push_back(line[i]);
	}
}

int KochSnowflake::groupingSize() const
{
	//Draw all vertices in one glDrawArrays call
	return -1;
}

Colour KochSnowflake::vertexColour(int vertex)
{
	if (vertex % 8 < 2)
	{
		return lineColours[0];
	}
	else if (vertex % 8 < 4)
	{
		return lineColours[1];
	}
	else if (vertex % 8 < 6)
	{
		return lineColours[2];
	}
	return lineColours[3];
}
