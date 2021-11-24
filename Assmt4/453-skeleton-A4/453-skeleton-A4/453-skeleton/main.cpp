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
const float sunSize = 696340.0f; //km
const float sunDisplaySize = 0.3f;
const float sphereParameterStep = 0.1f;
const float earthSize = 6371.0f; //km
const float earthToSun = 147.71e6; //km
const float moonSize = 1737.4f; //km
const float moonToEarth = 384400 * 10; //km
const float backdropSphereSize = sunSize;

// We gave this code in one of the tutorials, so leaving it here too
void updateGPUGeometry(GPU_Geometry& gpuGeom, CPU_Geometry const& cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setTexCoords(cpuGeom.texCoords);
	gpuGeom.setNormals(cpuGeom.normals);
}

class Planet //Includes sun and moon 
{
public:
	Planet(float actualSize, const char* texturePath, Planet* parent = NULL, float actualDistanceFromParent = 0.0f, float rotationAngle = PI/2)
		: size_((actualSize /sunSize) * sunDisplaySize)
		, distanceFromParent_((actualDistanceFromParent / sunSize) * sunDisplaySize / 100)
		, rotationAngle_(rotationAngle)
		, parent_(parent)
		, texture_(texturePath, GL_NEAREST)
	{
		if (parent_ != NULL)
		{
			size_ *= 20;
		}
	}

	void generateGeometry(bool invertNormals)
	{
		generateSphere(size_, sphereParameterStep, location(), invertNormals);
		updateGeometry();
	}

	glm::vec3 location() const
	{
		if (parent_ != NULL)
		{
			glm::vec3 positionRelativeToParent = { distanceFromParent_ * cos(rotationAngle_), 0.0f, -distanceFromParent_ * sin(rotationAngle_) };
			return positionRelativeToParent + parent_->location();
		}
		else
		{
			return glm::vec3(0.0f, 0.0f, 0.0f);
		}
	}

	void updateGeometry()
	{
		updateGPUGeometry(gpuGeom_, cpuGeom_);
	}

	void draw()
	{
		gpuGeom_.bind();
		texture_.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(cpuGeom_.verts.size()));
		texture_.unbind();
	}

	float actualSize()
	{
		return (size_ / sunDisplaySize) * sunSize;
	}

private:
	glm::vec3 generatePerVertexNormal(glm::vec3 vertex, glm::vec3 sphereCentre, bool invertNormals)
	{
		glm::vec3 normalVector;
		if (invertNormals)
		{
			normalVector = sphereCentre - vertex;
		}
		else
		{
			normalVector = vertex - sphereCentre;
		}
		return normalVector / glm::length(normalVector);
	}

	glm::vec3 findSphericalCoordinate(float radius, float theta, float phi)
	{
		return glm::vec3(radius * cos(theta) * sin(phi), radius * sin(theta) * sin(phi), radius * cos(phi));
	}

	glm::vec2 findTextureCoordinate(float theta, float phi)
	{
		return glm::vec2(theta / (2 * PI), phi / PI);
	}

	void generateSphere(float radius, float step, glm::vec3 sphereTranslation, bool invertNormals)
	{
		cpuGeom_.verts.clear();
		cpuGeom_.normals.clear();

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

				glm::vec3 bothIncremented = findSphericalCoordinate(radius, theta + step, phi + step) + sphereTranslation;

				cpuGeom_.verts.push_back(phiIncremented);
				cpuGeom_.verts.push_back(currentPoint);
				cpuGeom_.verts.push_back(thetaIncremented);

				cpuGeom_.texCoords.push_back(findTextureCoordinate(theta, phi + step));
				cpuGeom_.texCoords.push_back(findTextureCoordinate(theta, phi));
				cpuGeom_.texCoords.push_back(findTextureCoordinate(theta + step, phi));

				cpuGeom_.verts.push_back(bothIncremented);
				cpuGeom_.verts.push_back(phiIncremented);
				cpuGeom_.verts.push_back(thetaIncremented);

				cpuGeom_.texCoords.push_back(findTextureCoordinate(theta + step, phi + step));
				cpuGeom_.texCoords.push_back(findTextureCoordinate(theta, phi + step));
				cpuGeom_.texCoords.push_back(findTextureCoordinate(theta + step, phi));

				std::vector<glm::vec3> adjacents = { phiIncremented, phiIncrementedThetaDecremented, thetaDecremented, phiDecremented, thetaIncrementedPhiDecremented, thetaIncremented };
				cpuGeom_.normals.push_back(generatePerVertexNormal(phiIncremented, sphereTranslation, invertNormals));
				cpuGeom_.normals.push_back(generatePerVertexNormal(currentPoint, sphereTranslation, invertNormals));
				cpuGeom_.normals.push_back(generatePerVertexNormal(thetaIncremented, sphereTranslation, invertNormals));
				cpuGeom_.normals.push_back(generatePerVertexNormal(bothIncremented, sphereTranslation, invertNormals));
				cpuGeom_.normals.push_back(generatePerVertexNormal(phiIncremented, sphereTranslation, invertNormals));
				cpuGeom_.normals.push_back(generatePerVertexNormal(thetaIncremented, sphereTranslation, invertNormals));
			}
		}
	}

private:
	float size_;
	const Planet const* parent_;
	float distanceFromParent_;
	float rotationAngle_;
	Texture texture_;
	CPU_Geometry cpuGeom_;
	GPU_Geometry gpuGeom_;
};

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4() : camera(0.0, 0.0, 2.0), aspect(1.0f) {}

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

	Planet sun(sunSize, "textures/sunmap.jpg");
	sun.generateGeometry(false);

	Planet earth(earthSize, "textures/earthmap1k.jpg", &sun, earthToSun);
	earth.generateGeometry(false);

	//Source didn't have a moon texture map, so using pluto texture map for moon
	//Also scaling up the size of the moon and the distance to earth these values are so small relative
	//to the size of the sun
	Planet moon(moonSize * 2, "textures/plutomap1k.jpg", &earth, moonToEarth * 10);
	moon.generateGeometry(false);

	Planet starBackdrop((sunSize / 0.3f) * 20, "textures/starfield.jpg");
	starBackdrop.generateGeometry(true);

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

		sun.draw();
		earth.draw();
		moon.draw();
		starBackdrop.draw();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
