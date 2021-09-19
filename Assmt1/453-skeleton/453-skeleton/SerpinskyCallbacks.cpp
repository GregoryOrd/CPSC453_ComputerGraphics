#include "SerpinskyCallbacks.h"

SerpinskyCallbacks::SerpinskyCallbacks(SerpinskyTriangle& serpinskyTriangle, GPU_Geometry& gpuGeom)
	: serpinskyTriangle_(serpinskyTriangle)
	, gpuDrawer_(gpuGeom, serpinskyTriangle_.vertices())
{
}

void SerpinskyCallbacks::keyCallback(int key, int scancode, int action, int mods) {
	bool triangleChanged = false;
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		triangleChanged = serpinskyTriangle_.incrementIterations();
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		triangleChanged = serpinskyTriangle_.decrementIterations();
	}

	if (triangleChanged)
	{
		gpuDrawer_.drawSerpinskyVertices(serpinskyTriangle_.vertices());
	}
}
