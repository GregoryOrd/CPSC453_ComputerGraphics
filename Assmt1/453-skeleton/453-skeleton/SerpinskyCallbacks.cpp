#include "SerpinskyCallbacks.h"

SerpinskyCallbacks::SerpinskyCallbacks(std::vector<A_RecursiveShapeScene*>& sceneShapes, GPU_Geometry& gpuGeom, int* currentSceneNumber)
	: sceneShapes_(sceneShapes)
	, gpuDrawer_(gpuGeom, sceneShapes_.at(0)->vertices())
	, currentSceneNumber_(currentSceneNumber)
{
}

void SerpinskyCallbacks::keyCallback(int key, int scancode, int action, int mods) {
	bool sceneNumberUpdated = updateSceneNumber(key, action);
	bool iterationNumberUpdated = updateIteration(key, action);

	if (sceneNumberUpdated || iterationNumberUpdated)
	{
		gpuDrawer_.drawSerpinskyVertices(sceneShapes_.at(*currentSceneNumber_)->vertices());
	}
}

bool SerpinskyCallbacks::updateSceneNumber(int key, int action)
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

bool SerpinskyCallbacks::updateIteration(int key, int action)
{
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		return sceneShapes_.at(*currentSceneNumber_)->incrementIterations();
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		return sceneShapes_.at(*currentSceneNumber_)->decrementIterations();
	}
}
