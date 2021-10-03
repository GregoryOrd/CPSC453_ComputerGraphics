#include "GPUDrawer.h"

GPUDrawer::GPUDrawer(GPU_Geometry& gpuGeom, A_RecursiveShapeScene& initialScene, std::map<int, int> sceneNumberToGLPrimitiveMap)
	: gpuGeom_(gpuGeom)
	, sceneNumberToGLPrimitiveMap_(sceneNumberToGLPrimitiveMap)
{
	loadVerticesToGPU(initialScene);
}

void GPUDrawer::loadVerticesToGPU(A_RecursiveShapeScene& sceneShape)
{
	//Clear new CPU Geometry
	CPU_Geometry cpuGeom;
	std::vector<Point> vertices = sceneShape.vertices();

	// Load new serpinsky vertices into the CPU Geometry
	for (int i = 0; i < vertices.size(); i++)
	{
		Point vertex = vertices.at(i);
		int triangleNum = i / 3;

		Colour red = { 1.0f, 0.f, 0.f };
		cpuGeom.verts.push_back(vertex);
		cpuGeom.cols.push_back(sceneShape.vertexColour(i));
	}

	// Push the CPU Geometry onto the GPU Geometry
	gpuGeom_.setVerts(cpuGeom.verts);
	gpuGeom_.setCols(cpuGeom.cols);
}

void GPUDrawer::draw(int sceneNumber, int groupingSize, int numVertices)
{
	if (groupingSize == -1)
	{
		glDrawArrays(sceneNumberToGLPrimitiveMap_[sceneNumber], 0, GLsizei(numVertices));
	}
	else
	{
		for (int i = 0; i < numVertices / groupingSize; i++)
		{
			glDrawArrays(sceneNumberToGLPrimitiveMap_[sceneNumber], i * groupingSize, GLsizei(groupingSize));
		}
	}
}
