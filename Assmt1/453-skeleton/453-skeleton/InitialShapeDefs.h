#pragma once

#include <glm/glm.hpp>
#include <vector>
typedef glm::vec3 Point;

namespace InitialShapeDefs
{
	std::vector<Point> originalTrianglePositions = {
		Point(-0.5f, -0.5f, 0.f),
		Point(0.5f, -0.5f, 0.f),
		Point(0.f, 0.5f, 0.f)
	};
}
