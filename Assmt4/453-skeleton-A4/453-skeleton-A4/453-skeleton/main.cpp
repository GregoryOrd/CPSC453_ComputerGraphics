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
	gpuGeom.setTexCoords(cpuGeom.texCoords);
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

glm::vec3 generatePerVertexNormal(glm::vec3 vertex, glm::vec3 sphereCentre)
{
	glm::vec3 normalVector = vertex - sphereCentre;
	return normalVector / glm::length(normalVector);
}

glm::vec3 findSphericalCoordinate(float radius, float theta, float phi)
{
	return glm::vec3(radius* cos(theta)* sin(phi), radius* sin(theta)* sin(phi), radius* cos(phi));
}

glm::vec2 findTextureCoordinate(float theta, float phi)
{
	return glm::vec2(theta / (2 * PI), phi / PI);
}

void generateSphere(CPU_Geometry& sphere, float radius, float step, glm::vec3 sphereTranslation)
{
	sphere.verts.clear();
	sphere.normals.clear();

	for (float theta = 0.0f; theta <= 2 * PI; theta += step)
	{
		for (float phi = 0.0f; phi <= PI; phi += step)
		{
			glm::vec3 currentPoint = findSphericalCoordinate(radius, theta, phi) + sphereTranslation;

			glm::vec3 phiIncremented = findSphericalCoordinate(radius, theta, phi + step) + sphereTranslation;
			glm::vec3 phiDecremented = findSphericalCoordinate(radius, theta, phi - step) + sphereTranslation;

			glm::vec3 thetaIncremented = findSphericalCoordinate(radius, theta + step, phi) + sphereTranslation;
			glm::vec3 thetaDecremented = findSphericalCoordinate(radius, theta - step, phi) + sphereTranslation;

			glm::vec3 phiIncrementedThetaDecremented = findSphericalCoordinate(radius, theta - step, phi + step) + sphereTranslation;
			glm::vec3 thetaIncrementedPhiDecremented = findSphericalCoordinate(radius, theta + step, phi - step) + sphereTranslation;

			glm::vec3 bothIncremented =  findSphericalCoordinate(radius, theta + step, phi + step) + sphereTranslation;

			sphere.verts.push_back(phiIncremented);
			sphere.verts.push_back(currentPoint);
			sphere.verts.push_back(thetaIncremented);

			sphere.texCoords.push_back(findTextureCoordinate(theta, phi + step));
			sphere.texCoords.push_back(findTextureCoordinate(theta, phi));
			sphere.texCoords.push_back(findTextureCoordinate(theta + step, phi));

			sphere.verts.push_back(bothIncremented);
			sphere.verts.push_back(phiIncremented);
			sphere.verts.push_back(thetaIncremented);

			sphere.texCoords.push_back(findTextureCoordinate(theta + step, phi + step));
			sphere.texCoords.push_back(findTextureCoordinate(theta, phi + step));
			sphere.texCoords.push_back(findTextureCoordinate(theta + step, phi));

			std::vector<glm::vec3> adjacents = { phiIncremented, phiIncrementedThetaDecremented, thetaDecremented, phiDecremented, thetaIncrementedPhiDecremented, thetaIncremented };
			sphere.normals.push_back(generatePerVertexNormal(phiIncremented, sphereTranslation));
			sphere.normals.push_back(generatePerVertexNormal(currentPoint, sphereTranslation));
			sphere.normals.push_back(generatePerVertexNormal(thetaIncremented, sphereTranslation));
			sphere.normals.push_back(generatePerVertexNormal(bothIncremented, sphereTranslation));
			sphere.normals.push_back(generatePerVertexNormal(phiIncremented, sphereTranslation));
			sphere.normals.push_back(generatePerVertexNormal(thetaIncremented, sphereTranslation));
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

	glm::vec3 sphereTranslation = { 0.0f, 0.0f, 0.0f };

	CPU_Geometry sphere;
	generateSphere(sphere, 0.25f, 0.1f, sphereTranslation);

	GPU_Geometry sphereGpuGeom;
	updateGPUGeometry(sphereGpuGeom, sphere);

	Texture sphereTexture("textures/earthmap1k.jpg", GL_NEAREST);

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

		sphereGpuGeom.bind();
		sphereTexture.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sphere.verts.size()));
		sphereTexture.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
