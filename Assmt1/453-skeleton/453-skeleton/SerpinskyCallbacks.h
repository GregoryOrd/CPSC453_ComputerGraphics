#pragma once

#include "Window.h"
#include "A_RecursiveShapeScene.h"
#include "SerpinskyGPUDrawer.h"
#include "Geometry.h"

class SerpinskyCallbacks : public CallbackInterface
{
public:
	SerpinskyCallbacks(std::vector<A_RecursiveShapeScene*>& sceneShapes, GPU_Geometry& gpuGeom, int* currentSceneNumber);

	virtual void keyCallback(int key, int scancode, int action, int mods);

private:
	bool updateSceneNumber(int key, int action);
	bool updateIteration(int key, int action);

private:
	std::vector<A_RecursiveShapeScene*>& sceneShapes_;
	SerpinskyGPUDrawer gpuDrawer_;
	int* currentSceneNumber_;
};
