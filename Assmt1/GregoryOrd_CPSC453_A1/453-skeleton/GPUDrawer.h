#pragma once

#include "Geometry.h"
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "A_RecursiveShapeScene.h"

typedef glm::vec3 Point;
typedef glm::vec3 Colour;

class GPUDrawer
{
public:
	GPUDrawer(GPU_Geometry& gpuGeom, A_RecursiveShapeScene& initialScene, std::map<int, int> sceneNumberToGLPrimitiveMap);

	void loadVerticesToGPU(A_RecursiveShapeScene& sceneShape);
	void draw(int sceneNumber, int groupingSize, int numVertices);
private:
	GPU_Geometry& gpuGeom_;
	std::map<int, int> sceneNumberToGLPrimitiveMap_;
};
