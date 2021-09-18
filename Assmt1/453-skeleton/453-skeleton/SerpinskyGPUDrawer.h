#pragma once

#include "Geometry.h"
#include <vector>
#include <glm/glm.hpp>

typedef glm::vec3 Point;
typedef glm::vec3 Colour;

static const Colour triangleColours[3] = {
	Colour(1.f, 0.f, 0.f), // top triangle          - red
	Colour(0.f, 1.f, 0.f), // bottom left triangle  - green
	Colour(0.f, 0.f, 1.f)  // bottom right triangle - blue
};

class SerpinskyGPUDrawer
{
public:
	SerpinskyGPUDrawer(GPU_Geometry& gpuGeom, std::vector<Point> initialSerpinksyVertices);

	void drawSerpinskyVertices(std::vector<Point> serpinksyVertices);
private:
	GPU_Geometry& gpuGeom_;
};
