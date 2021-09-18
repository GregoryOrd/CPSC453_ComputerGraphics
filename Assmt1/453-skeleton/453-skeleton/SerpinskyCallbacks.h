#pragma once

#include "Window.h"
#include "SerpinskyTriangle.h"
#include "SerpinskyGPUDrawer.h"
#include "Geometry.h"

class SerpinskyCallbacks : public CallbackInterface
{
public:
	SerpinskyCallbacks(SerpinskyTriangle& serpinskyTriangle, GPU_Geometry& gpuGeom);

	virtual void keyCallback(int key, int scancode, int action, int mods);

private:
	SerpinskyTriangle& serpinskyTriangle_;
	SerpinskyGPUDrawer gpuDrawer_;
};
