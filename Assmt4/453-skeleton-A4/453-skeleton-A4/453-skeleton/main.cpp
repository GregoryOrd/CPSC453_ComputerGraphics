#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#define PI 3.14159265359

// We gave this code in one of the tutorials, so leaving it here too
void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
	gpuGeom.setNormals(cpuGeom.normals);
}

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4() : camera(0.0, 0.0, 2.0), aspect(1.0f) {
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS) {
				rightMouseDown = true;
			} else if (action == GLFW_RELEASE) {
				rightMouseDown = false;
			}
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			double dx = xpos - mouseOldX;
			double dy = ypos - mouseOldY;
			camera.incrementTheta(dy);
			camera.incrementPhi(dx);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp) {
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);

		GLint location = glGetUniformLocation(sp, "light");
		glm::vec3 light = camera.getPos();
		glUniform3fv(location, 1, glm::value_ptr(light));

		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}

	Camera camera;

private:

	bool rightMouseDown = false;
	float aspect;
	double mouseOldX;
	double mouseOldY;

};

void colouredTriangles(CPU_Geometry &geom) {
	geom.cols.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.cols.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.cols.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.cols.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.cols.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.cols.push_back(glm::vec3(1.0, 0.0, 0.0));
}

void positiveZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom) {
	const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.5));
	for(auto i = originQuad.begin(); i < originQuad.end(); ++i) {
		geom.verts.push_back(
			glm::vec3(T * glm::vec4((*i), 1.0))
		);
	}
	geom.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, 1.0));
}

void positiveXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom) {
	const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
	for(auto i = originQuad.begin(); i < originQuad.end(); ++i) {
		geom.verts.push_back(
			glm::vec3(T * R * glm::vec4((*i), 1.0))
		);
	}
	geom.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(1.0, 0.0, 0.0));
}

void negativeZFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom) {
	const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.5f));
	for(auto i = originQuad.begin(); i < originQuad.end(); ++i) {
		geom.verts.push_back(
			glm::vec3(T * R * glm::vec4((*i), 1.0))
		);
	}
	geom.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
	geom.normals.push_back(glm::vec3(0.0, 0.0, -1.0));
}

void negativeXFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom) {
	const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f));
	for(auto i = originQuad.begin(); i < originQuad.end(); ++i) {
		geom.verts.push_back(
			glm::vec3(T * R * glm::vec4((*i), 1.0))
		);
	}
	geom.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
	geom.normals.push_back(glm::vec3(-1.0, 0.0, 0.0));
}

void positiveYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom) {
	const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f));
	for(auto i = originQuad.begin(); i < originQuad.end(); ++i) {
		geom.verts.push_back(
			glm::vec3(T * R * glm::vec4((*i), 1.0))
		);
	}
	geom.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, 1.0, 0.0));
}

void negativeYFace(std::vector<glm::vec3> const &originQuad, CPU_Geometry &geom) {
	const glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	const glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f));
	for(auto i = originQuad.begin(); i < originQuad.end(); ++i) {
		geom.verts.push_back(
			glm::vec3(T * R * glm::vec4((*i), 1.0))
		);
	}
	geom.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
	geom.normals.push_back(glm::vec3(0.0, -1.0, 0.0));
}

glm::vec3 generatePerVertexNormal(glm::vec3 vertex/*, std::vector<glm::vec3> adjacentVertexes*/)
{
	/*glm::vec3 phiIncremented = adjacentVertexes[0];
	glm::vec3 phiIncrementedThetaDecremented = adjacentVertexes[1];
	glm::vec3 thetaDecremented = adjacentVertexes[2];
	glm::vec3 phiDecremented = adjacentVertexes[3];
	glm::vec3 thetaIncrementedPhiDecremented = adjacentVertexes[4];
	glm::vec3 thetaIncremented = adjacentVertexes[5];

	glm::vec3 adjacentVector1 = phiIncremented - vertex;
	glm::vec3 adjacentVector2 = phiIncrementedThetaDecremented - vertex;
	glm::vec3 adjacentVector3 = thetaDecremented - vertex;
	glm::vec3 adjacentVector4 = phiDecremented - vertex;
	glm::vec3 adjacentVector5 = thetaIncrementedPhiDecremented - vertex;
	glm::vec3 adjacentVector6 = thetaIncremented - vertex;

	std::vector<glm::vec3> adjacentNormals;
	adjacentNormals.push_back(glm::cross(adjacentVector1, adjacentVector2));
	adjacentNormals.push_back(glm::cross(adjacentVector2, adjacentVector3));
	adjacentNormals.push_back(glm::cross(adjacentVector3, adjacentVector4));
	adjacentNormals.push_back(glm::cross(adjacentVector4, adjacentVector5));
	adjacentNormals.push_back(glm::cross(adjacentVector5, adjacentVector6));
	adjacentNormals.push_back(glm::cross(adjacentVector6, adjacentVector1));

	glm::vec3 vertexNormal = { 0.0f, 0.0f, 0.0f };

	for (glm::vec3 normal : adjacentNormals)
	{
		vertexNormal = vertexNormal + normal;
	}
	return vertexNormal / 6.0f;*/
	//return glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 origin = { 0.0f, 0.0f, 0.0f };
	return vertex - origin;
}

glm::vec3 findSphericalCoordinate(float radius, float theta, float phi)
{
	return glm::vec3(radius* cos(theta)* sin(phi), radius* sin(theta)* sin(phi), radius* cos(phi));
}

void generateSphere(CPU_Geometry& sphere)
{
	sphere.verts.clear();
	sphere.normals.clear();

	float radius = 0.5f;
	float step = 0.1f;
	for (float theta = 0.0f; theta <= 2 * PI; theta += step)
	{
		for (float phi = 0.0f; phi <= PI; phi += step)
		{
			glm::vec3 currentPoint = findSphericalCoordinate(radius, theta, phi);

			glm::vec3 phiIncremented = findSphericalCoordinate(radius, theta, phi + step);
			glm::vec3 phiDecremented = findSphericalCoordinate(radius, theta, phi - step);

			glm::vec3 thetaIncremented = findSphericalCoordinate(radius, theta + step, phi);
			glm::vec3 thetaDecremented = findSphericalCoordinate(radius, theta - step, phi);

			glm::vec3 phiIncrementedThetaDecremented = findSphericalCoordinate(radius, theta - step, phi + step);
			glm::vec3 thetaIncrementedPhiDecremented = findSphericalCoordinate(radius, theta + step, phi - step);

			glm::vec3 bothIncremented =  findSphericalCoordinate(radius, theta + step, phi + step);

			sphere.verts.push_back(phiIncremented);
			sphere.verts.push_back(currentPoint);
			sphere.verts.push_back(thetaIncremented);

			sphere.verts.push_back(bothIncremented);
			sphere.verts.push_back(phiIncremented);
			sphere.verts.push_back(thetaIncremented);

			std::vector<glm::vec3> adjacents = { phiIncremented, phiIncrementedThetaDecremented, thetaDecremented, phiDecremented, thetaIncrementedPhiDecremented, thetaIncremented };
			sphere.normals.push_back(generatePerVertexNormal(phiIncremented));
			sphere.normals.push_back(generatePerVertexNormal(currentPoint));
			sphere.normals.push_back(generatePerVertexNormal(thetaIncremented));
			sphere.normals.push_back(generatePerVertexNormal(bothIncremented));
			sphere.normals.push_back(generatePerVertexNormal(phiIncremented));
			sphere.normals.push_back(generatePerVertexNormal(thetaIncremented));
		}
	}
}


int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);


	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// The current CPU_Geometry and GPU_Geometry classes are defined in
	// Geometry.h/Geometry.cpp They will work for this assignment, but for some of
	// the bonuses you may have to modify them.

	std::vector<glm::vec3> originQuad;
	originQuad.push_back(glm::vec3{-0.5, 0.5, 0}); // top-left
	originQuad.push_back(glm::vec3{-0.5, -0.5, 0}); // bottom-left
	originQuad.push_back(glm::vec3{0.5, 0.5, 0}); // top-right

	originQuad.push_back(glm::vec3{-0.5, -0.5, 0}); // bottom-left
	originQuad.push_back(glm::vec3{0.5, -0.5, 0}); // bottom-right
	originQuad.push_back(glm::vec3{0.5, 0.5, 0}); // top-right

	CPU_Geometry square;
	/*positiveZFace(originQuad, square);
	positiveXFace(originQuad, square);
	negativeZFace(originQuad, square);
	negativeXFace(originQuad, square);
	positiveYFace(originQuad, square);
	negativeYFace(originQuad, square);*/
	generateSphere(square);

	square.cols.resize(square.verts.size(), glm::vec3{1.0, 0.0, 0.0});
	std::cout << "The sphere has " << square.verts.size() << " verts" << std::endl;
	std::cout << "The sphere has " << square.normals.size() << " normals" << std::endl;
	/*colouredTriangles(square);
	colouredTriangles(square);
	colouredTriangles(square);
	colouredTriangles(square);
	colouredTriangles(square);
	colouredTriangles(square);*/


	/*for(auto i = square.verts.begin(); i < square.verts.end(); ++i) {
		std::cout << *i << std::endl;
	}*/

	GPU_Geometry quads;
	updateGPUGeometry(quads, square);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		shader.use();

		a4->viewPipeline(shader);

		quads.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(square.verts.size()));

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
