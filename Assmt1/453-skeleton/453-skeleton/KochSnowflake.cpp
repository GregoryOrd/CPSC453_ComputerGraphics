#include "KochSnowflake.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

KochSnowflake::KochSnowflake(std::vector<Point>& originalLine, int minIterations, int maxIterations)
	: A_RecursiveShapeScene(originalLine, minIterations, maxIterations)
{
	findShapesRecursively(originalLine, minIterations);
}

void KochSnowflake::findShapesRecursively(std::vector<Point>& line, int iterations)
{
	if (iterations == 0)
	{
		pushShapesIntoVertices(line);
		return;
	}

	Point pointA = line[0];
	Point pointB = line[1];
	Point pointC = (2.f / 3.f) * pointA + (1.f / 3.f) * pointB;
	Point pointD = (1.f / 3.f) * pointA + (2.f / 3.f) * pointB;
	Point pointE;

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
	else if(vectorAB[1] < 0)
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

	if (iterations > 1)
	{
		findShapesRecursively(flatLeft, iterations - 1);
		findShapesRecursively(angledLeft, iterations - 1);
		findShapesRecursively(angledRight, iterations - 1);
		findShapesRecursively(flatRight, iterations - 1);
	}
	else
	{
		pushShapesIntoVertices(flatLeft);
		pushShapesIntoVertices(angledLeft);
		pushShapesIntoVertices(angledRight);
		pushShapesIntoVertices(flatRight);
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
