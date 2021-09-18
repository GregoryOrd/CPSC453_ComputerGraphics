#include "SerpinskyGPUDrawer.h"

SerpinskyGPUDrawer::SerpinskyGPUDrawer(GPU_Geometry& gpuGeom, std::vector<Point> initialSerpinksyVertices)
	: gpuGeom_(gpuGeom)
{
	drawSerpinskyVertices(initialSerpinksyVertices);
}

void SerpinskyGPUDrawer::drawSerpinskyVertices(std::vector<Point> serpinksyVertices)
{
	//Clear previous CPU Geometry
	CPU_Geometry cpuGeom;
	cpuGeom.verts.clear();
	cpuGeom.cols.clear();

	// Load new serpinsky vertices into the CPU Geometry
	for (int i = 0; i < serpinksyVertices.size(); i++)
	{
		Point vertex = serpinksyVertices.at(i);
		int triangleNum = i / 3;

		cpuGeom.verts.push_back(vertex);
		cpuGeom.cols.push_back(triangleColours[triangleNum % 3]);
	}

	// Push the CPU Geometry onto the GPU Geometry
	gpuGeom_.setVerts(cpuGeom.verts);
	gpuGeom_.setCols(cpuGeom.cols);
}
