#include "DragonCurve.h"
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

DragonCurve::DragonCurve(std::vector<Point>& originalLine, int minIterations, int maxIterations)
	: A_RecursiveShapeScene(originalLine, minIterations, maxIterations)
{
	findShapesRecursively(originalLine, minIterations);
}

Point DragonCurve::markPoint(float sideLength, Point startingPoint, float angleFromXAxis)
{
	float xMovement = sideLength * cos(angleFromXAxis);
	float yMovement = sideLength * sin(angleFromXAxis);
	Point newPoint = { startingPoint[0] + xMovement, startingPoint[1] + yMovement, 0.f };
	vertices_.push_back(newPoint);
	return newPoint;
}

void DragonCurve::dragonCurveRecursion(int iteration, float sideLength, DIRECTION direction, Point startingPoint, float startingAngleFromXAxis)
{
	if (iteration == 0)
	{
		markPoint(sideLength, startingPoint, startingAngleFromXAxis);
		return;
	}

	//Rotate angle 45 degress
	float newAngle = startingAngleFromXAxis;
	if (direction == RIGHT)
	{
		newAngle -= M_PI / 4;
	}
	else
	{
		newAngle += M_PI / 4;
	}

	float newSideLength = sideLength * (sqrt(2) / 2);
	dragonCurveRecursion(iteration - 1, newSideLength, RIGHT, startingPoint, newAngle);
	Point newPoint = markPoint(newSideLength, startingPoint, newAngle);

	newAngle = startingAngleFromXAxis;
	if (direction == RIGHT)
	{
		newAngle += M_PI / 4;
	}
	else
	{
		newAngle -= M_PI / 4;
	}

	dragonCurveRecursion(iteration - 1, newSideLength, LEFT, newPoint, newAngle);
}

void DragonCurve::findShapesRecursively(std::vector<Point>& line, int iterations)
{
	float lineX = line[1][0] - line[0][0];
	float lineY = line[1][1] - line[0][1];
	float initialSideLength = sqrt(lineX * lineX + lineY * lineY);

	Point lineVector = line[1] - line[0];
	float angleFromXAxis = atan(lineVector[1] / lineVector[0]);

	vertices_.push_back(line[0]);
	dragonCurveRecursion(iterations, initialSideLength, RIGHT, line[0], angleFromXAxis);
}

void DragonCurve::pushShapesIntoVertices(std::vector<Point>& line)
{
	for (int i = 0; i < 2; i++)
	{
		vertices_.push_back(line[i]);
	}
}

int DragonCurve::groupingSize() const
{
	//Draw all vertices in one glDrawArrays call
	return -1;
}

Colour DragonCurve::vertexColour(int vertex)
{
	Colour red = { 1.0f, 0.0f, 0.0f };
	Colour green = { 0.f, 1.f, 0.f };
	if ((vertex / 2) % 2 == 0)
	{
		return red;
	}
	return green;
}
