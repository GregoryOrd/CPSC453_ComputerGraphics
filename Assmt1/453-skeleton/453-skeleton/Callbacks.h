#pragma once

#include "Window.h"
#include "A_RecursiveShapeScene.h"
#include "GPUDrawer.h"

class Callbacks : public CallbackInterface
{
public:
	Callbacks(std::vector<A_RecursiveShapeScene*>& sceneShapes, GPUDrawer& gpuDrawer, int* initialSceneNumber);

	virtual void keyCallback(int key, int scancode, int action, int mods);

private:
	bool updateSceneNumber(int key, int action);
	bool updateIteration(int key, int action);

private:
	std::vector<A_RecursiveShapeScene*>& sceneShapes_;
	GPUDrawer gpuDrawer_;
	int* currentSceneNumber_;
};
