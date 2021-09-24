#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Geometry.h"
#include "GLDebug.h"
#include "KochSnowflake.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"
#include "SerpinskyTriangle.h"
#include "SerpinskyGPUDrawer.h"
#include "SquaresAndDiamonds.h"
#include "SerpinskyCallbacks.h"
#include "InitialShapeDefs.h"
#include "A_RecursiveShapeScene.h"

int main() {
	//Init
	Log::debug("Starting main");
	glfwInit();
	Window window(800, 800, "Serpinksy Triangle and Menger Sponge");
	GLDebug::enable();
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");
	int currentSceneNumber = 0;

	//Declare GPU Geometry Object
	GPU_Geometry gpuGeom;

	//Initialize the shapes for each scene
	std::vector<A_RecursiveShapeScene*> sceneShapes;
	sceneShapes.push_back(new SerpinskyTriangle(InitialShapeDefs::originalTrianglePositions, 0, 10));
	sceneShapes.push_back(new SquaresAndDiamonds(InitialShapeDefs::originalSquarePositions, 0, 10));
	sceneShapes.push_back(new KochSnowflake(InitialShapeDefs::originalKochSnowflakeLine, 0, 8));
	sceneShapes.push_back(new SerpinskyTriangle(InitialShapeDefs::originalTrianglePositions, 0, 10));

	//Instantiate a drawer object with the initial SerpinskyTriangle vertices as the initial vertices.
	SerpinskyGPUDrawer drawer(gpuGeom, *sceneShapes[0], InitialShapeDefs::sceneNumberToGLPrimitive);

	//Setup Keyboard Callbacks to switch between scenes and
	//increment/decrement number of iterations in a scene
	window.setCallbacks(std::make_shared<SerpinskyCallbacks>(sceneShapes, drawer, &currentSceneNumber));

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		drawer.draw(currentSceneNumber, sceneShapes.at(currentSceneNumber)->groupingSize(), sceneShapes.at(currentSceneNumber)->numVertices());
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	// Cleanup the scene shape pointers
	while (sceneShapes.size() > 0)
	{
		sceneShapes.pop_back();
	}

	glfwTerminate();
	return 0;
}
