#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <map>
typedef glm::vec3 Point;

namespace InitialShapeDefs
{
	std::vector<Point> originalTrianglePositions = {
		Point(-0.5f, -0.5f, 0.f),
		Point(0.5f, -0.5f, 0.f),
		Point(0.f, 0.5f, 0.f)
	};

	std::vector<Point> originalSquarePositions = {
		Point(-0.5f, -0.5f, 0.f),
		Point(0.5f, -0.5f, 0.f),
		Point(0.5f, 0.5f, 0.f),
		Point(-0.5f, 0.5f, 0.f)
	};

	std::vector<Point> originalLine = {
		Point(-0.5f, 0.f, 0.f),
		Point(0.5f, 0.f, 0.f)
	};

	//
	//4.32978e-17, -0.707107

	std::map<int, int> sceneNumberToGLPrimitive = {
		{ 0, GL_TRIANGLES }, // Serpinsky Triangle
		{ 1, GL_LINE_LOOP }, // Squares and Diamonds
		{ 2, GL_LINES },     // Koch Snowflake
		{ 3, GL_LINE_STRIP }      // Dragon Curve
	};
}
