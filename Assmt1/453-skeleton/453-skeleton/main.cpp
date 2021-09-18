#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"
#include "SerpinskyTriangle.h"
#include "SerpinskyCallbacks.h"

typedef glm::vec3 Colour;
typedef Colour TriangleVertexColours[3];

int main() {
	Log::debug("Starting main");
	Log::info("This program starts with a standard triangle (aka 0 iterations). Use the up arrow key to increment or the down arrow key to decrement the number of iterations between 0 and 10.");
	glfwInit();
	Window window(800, 800, "Serpinksy Triangle and Menger Sponge");
	GLDebug::enable();
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// Defing intial triangle
	TrianglePositions originalTrianglePositions = {
		Point(-0.5f, -0.5f, 0.f),
		Point(0.5f, -0.5f, 0.f),
		Point(0.f, 0.5f, 0.f)
	};

	//Initialize the SerpinskyTriangle and GPU_Geometry data structures
	SerpinskyTriangle serpinskyTriangle(originalTrianglePositions, 0, 10);
	GPU_Geometry gpuGeom;

	//Setup Keyboard Callbacks to increment/decrement number of serpinksy iterations
	window.setCallbacks(std::make_shared<SerpinskyCallbacks>(serpinskyTriangle, gpuGeom));

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(serpinskyTriangle.numVertices()));
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
