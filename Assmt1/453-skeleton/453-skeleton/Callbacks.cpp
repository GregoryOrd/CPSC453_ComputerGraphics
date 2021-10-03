#include "Callbacks.h"

Callbacks::Callbacks(std::vector<A_RecursiveShapeScene*>& sceneShapes, GPUDrawer& gpuDrawer, int* initialSceneNumber)
	: sceneShapes_(sceneShapes)
	, gpuDrawer_(gpuDrawer)
	, currentSceneNumber_(initialSceneNumber)
{
}

void Callbacks::keyCallback(int key, int scancode, int action, int mods) {
	bool sceneNumberUpdated = updateSceneNumber(key, action);
	bool iterationNumberUpdated = updateIteration(key, action);

	if (sceneNumberUpdated || iterationNumberUpdated)
	{
		gpuDrawer_.loadVerticesToGPU(*sceneShapes_.at(*currentSceneNumber_));
	}
}

bool Callbacks::updateSceneNumber(int key, int action)
{
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS && *currentSceneNumber_ < 3) {
		(*currentSceneNumber_)++;
		return true;
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS && *currentSceneNumber_ > 0) {
		(*currentSceneNumber_)--;
		return true;
	}
	return false;
}

bool Callbacks::updateIteration(int key, int action)
{
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		return sceneShapes_.at(*currentSceneNumber_)->incrementIterations();
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		return sceneShapes_.at(*currentSceneNumber_)->decrementIterations();
	}
}
