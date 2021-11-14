#include <GL/glew.h>
#include <GL/glu.h>
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

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#define PI 3.14159265359

const glm::vec3 selectedColour = { 0.f, 0.f, 1.0f };
const glm::vec3 nonSelectedColour = { 1.f, 0.0f, 0.0f };
const glm::vec3 generatedCurveColour = { 0.f, 0.0f, 1.0f };
const float collisionThreshold = 0.01f;
const int numPointsOnGeneratedCurve = 50;
bool bsplineGeneratedForSurface = false;
bool isBezierCurve = true;
bool showControlPolygon = true;
bool showControlPoints = true;
bool showFloorGrid = true;
bool wireFrame = true;
int sceneNumber = 0;
const float cameraTranslationIncrement = 0.01f;
const float floorGridStep = 0.05f;
CPU_Geometry bsplineCurve_;

const glm::vec3 initialEye = { 0.0f, 0.0f, 2.0f };
const glm::vec3 initialCenter = { 0.0f, 0.0f, 0.0f };
const glm::vec3 initialUpVector = { 0.0f, -1.0f, 0.0f };

// We gave this code in one of the tutorials, so leaving it here too
void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}


class Camera
{
public:
	Camera(Window& window, glm::vec3 eye, glm::vec3 center, glm::vec3 upVector):
		fovy_(55.0f), aspect_(window.getWidth()/window.getHeight()), zNear_(1.0f), zFar_(-1.0f),
		eye_(eye), center_(center), upVector_(upVector)
	{
	}

	void moveForward()
	{
		eye_ = eye_ + cameraTranslationIncrement * direction();
	}
	void moveBackward()
	{
		eye_ = eye_ - cameraTranslationIncrement * direction();
	}
	void moveUp()
	{
		eye_ = eye_ - cameraTranslationIncrement * upVector_;
		center_ = center_ - cameraTranslationIncrement * upVector_;
	}
	void moveDown()
	{
		eye_ = eye_ + cameraTranslationIncrement * upVector_;
		center_ = center_ + cameraTranslationIncrement * upVector_;
	}
	void moveLeft()
	{
		eye_ = eye_ - cameraTranslationIncrement * rightDirection();
		center_ = center_ - cameraTranslationIncrement * rightDirection();
	}
	void moveRight()
	{
		eye_ = eye_ + cameraTranslationIncrement * rightDirection();
		center_ = center_ + cameraTranslationIncrement * rightDirection();
	}
	glm::mat4 perspectiveMatrix()
	{
		if (sceneNumber == 0)
		{
			return glm::mat4(1.0f);
		}
		else
		{
			return glm::perspective(fovy_, aspect_, zNear_, zFar_);
		}
	}
	glm::mat4 viewMatrix()
	{
		if (sceneNumber == 0)
		{
			return glm::mat4(1.0f);
		}
		else
		{
			return glm::lookAt(eye_, center_, upVector_);
		}
	}

	float lengthOfEyeToCenter()
	{
		return direction().length();
	}

	void setCenter(glm::vec3 center)
	{
		center_ = center;
	}

	glm::vec3 center()
	{
		return center_;
	}

	glm::vec3 eye()
	{
		return eye_;
	}

	glm::vec3 direction()
	{
		return center_ - eye_;
	}

	void reset()
	{
		eye_ = initialEye;
		center_ = initialCenter;
		upVector_ = initialUpVector;
	}

private:
	glm::vec3 rightDirection()
	{
		return glm::cross(upVector_, direction());
	}

private:
	float fovy_;
	float aspect_;
	float zNear_;
	float zFar_;
	glm::vec3 eye_;
	glm::vec3 center_;
	glm::vec3 upVector_;
};

class CursorPositionConverter
{
public:
	CursorPositionConverter(Window& window) : window_(window) {}

	double convertedXPos(double cursorXPos)
	{
		double halfWindowWidth = window_.getWidth() / 2;
		if (cursorXPos == halfWindowWidth)
		{
			return 0.0;
		}
		double distanceFromHalf = cursorXPos - halfWindowWidth;
		return distanceFromHalf / halfWindowWidth;
	}

	double convertedYPos(double cursorYPos)
	{
		double halfWindowHeight = window_.getHeight() / 2;
		if (cursorYPos == halfWindowHeight)
		{
			return 0.0;
		}
		double distanceFromHalf = halfWindowHeight - cursorYPos;
		return distanceFromHalf / halfWindowHeight;
	}

private:
	Window& window_;
};

// EXAMPLE CALLBACKS
class Assignment3 : public CallbackInterface {

public:
	Assignment3(CPU_Geometry& square, CPU_Geometry& generatedCurve, GPU_Geometry& generatedGPUGeom, GPU_Geometry& pointsGPUGeom, GPU_Geometry& linesGPUGeom, CursorPositionConverter& converter, glm::vec3* selectedPoint, Camera& camera)
		: square_(square), generatedCurve_(generatedCurve), generatedGPUGeom_(generatedGPUGeom), pointsGPUGeom_(pointsGPUGeom), linesGPUGeom_(linesGPUGeom), converter_(converter), xPos_(0.f), yPos_(0.f), mouseDragging_(false), selectedIndex_(-1), camera_(camera)
	{
	}

	void colourPointsAndSetSelectedIndex()
	{
		for (int i = 0; i < square_.verts.size(); i++)
		{
			colourPointAndSetSelectedIndex(i);
		}
	}

	void colourPointAndSetSelectedIndex(int i)
	{
		if (abs((square_.verts.at(i))[0] - convertedXPos()) <= collisionThreshold && abs((square_.verts.at(i))[1] - convertedYPos()) <= collisionThreshold)
		{
			square_.cols.at(i) = selectedColour;
			selectedIndex_ = i;
		}
		else
		{
			square_.cols.at(i) = nonSelectedColour;
		}
	}

	void addPoint()
	{
		square_.verts.push_back({ convertedXPos(), convertedYPos(), 0.0f });
		square_.cols.push_back(selectedColour);
		selectedIndex_ = -1;
		mouseDragging_ = false;
	}

	void updateGeneratedCurve()
	{
		updateGPUGeometry(generatedGPUGeom_, generatedCurve_);
	}

	void updatePoints()
	{
		// Reset the colors to red, selected point to blue
		square_.cols.clear();
		square_.cols.resize(square_.verts.size(), glm::vec3{ 1.0, 0.0, 0.0 });
		if (selectedIndex() != -1)
		{
			square_.cols.at(selectedIndex()) = glm::vec3{ 0.0f, 0.0f, 1.0f };
		}
		updateGPUGeometry(pointsGPUGeom_, square_);
	}

	void updateLines()
	{
		// Reset the colors to green
		square_.cols.clear();
		square_.cols.resize(square_.verts.size(), glm::vec3{ 0.0, 1.0, 0.0 });
		updateGPUGeometry(linesGPUGeom_, square_);
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (sceneNumber == 0)
		{
			if (key == GLFW_KEY_D && action == GLFW_PRESS)
			{
				if (selectedIndex() != -1)
				{
					square_.cols.erase(square_.cols.begin() + selectedIndex());
					square_.verts.erase(square_.verts.begin() + selectedIndex());
				}

				selectedIndex_ = -1;

				updatePoints();
				updateLines();
			}
			else if (key == GLFW_KEY_R && action == GLFW_PRESS)
			{

				square_.cols.clear();
				square_.verts.clear();

				generatedCurve_.cols.clear();
				generatedCurve_.verts.clear();

				selectedIndex_ = -1;

				updatePoints();
				updateLines();
				updateGeneratedCurve();
			}
			else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
			{
				isBezierCurve = !isBezierCurve;
			}
			else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
			{
				showControlPolygon = !showControlPolygon;
			}
			else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
			{
				showControlPoints = !showControlPoints;
			}
		}
		else
		{
			//3D Viewer Controls Here
			if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				camera_.moveForward();
			}
			else if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				camera_.moveBackward();
			}
			else if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				camera_.moveLeft();
			}
			else if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				camera_.moveRight();
			}
			else if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				camera_.moveUp();
			}
			else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
			{
				camera_.moveDown();
			}
			else if (key == GLFW_KEY_F && action == GLFW_PRESS)
			{
				showFloorGrid = !showFloorGrid;
			}
		}

		if (sceneNumber == 2)
		{
			if (key == GLFW_KEY_T && action == GLFW_PRESS)
			{
				wireFrame = !wireFrame;
			}
		}

		if (key == GLFW_KEY_G && action == GLFW_PRESS)
		{
			//2D Editor View
			sceneNumber = 0;
		}
		else if (key == GLFW_KEY_H && action == GLFW_PRESS)
		{
			//3D Curve Viewer
			sceneNumber = 1;
			camera_.reset();
		}
		else if (key == GLFW_KEY_J && action == GLFW_PRESS)
		{
			//3D Surface of Revolution Viewer
			sceneNumber = 2;
			camera_.reset();
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS && !mouseDragging_)
		{
			mouseDragging_ = true;
			dragStart_ = { convertedXPos(), convertedYPos() };

			if (sceneNumber == 0)
			{
				selectedIndex_ = -1;
				colourPointsAndSetSelectedIndex();

				if (selectedIndex_ == -1)
				{
					addPoint();
				}

				updatePoints();
				updateLines();
			}
		}
		else if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
		{
			mouseDragging_ = false;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) {
		xPos_ = xpos;
		yPos_ = ypos;
	}

	virtual void scrollCallback(double xoffset, double yoffset) {
	}

	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
	}

	bool mouseDragging()
	{
		return mouseDragging_;
	}

	float convertedXPos()
	{
		return converter_.convertedXPos(xPos_);
	}

	float convertedYPos()
	{
		return converter_.convertedYPos(yPos_);
	}

	int selectedIndex()
	{
		return selectedIndex_;
	}

	glm::vec2 dragStart()
	{
		return dragStart_;
	}

	void updateDragStart()
	{
		dragStart_ = { convertedXPos(), convertedYPos() };
	}

	Camera& camera()
	{
		return camera_;
	}

private:
	CursorPositionConverter& converter_;
	CPU_Geometry& square_;
	GPU_Geometry& pointsGPUGeom_;
	GPU_Geometry& linesGPUGeom_;
	CPU_Geometry& generatedCurve_;
	GPU_Geometry& generatedGPUGeom_;
	Camera& camera_;
	float xPos_;
	float yPos_;
	int selectedIndex_;
	bool mouseDragging_;
	glm::vec2 dragStart_;
};
void dragCamera(Assignment3& a3)
{
	if (a3.mouseDragging())
	{
		glm::vec2 dragVector = { a3.convertedXPos() - a3.dragStart()[0], a3.convertedYPos() - a3.dragStart()[1] };

		if (dragVector[0] != 0.0f)
		{
			glm::vec4 translatedCameraCenterHomogeneous(a3.camera().center() + a3.camera().direction(), 1.0f);

			float rotationAngle = dragVector[0] * PI / 2;
			glm::mat4 rotationAboutYAxis = {
				{cos(rotationAngle), 0.0f, sin(rotationAngle), 0.0f},
				{0.0f, 1.0f, 0.0f, 0.0f},
				{-sin(rotationAngle), 0.0f, cos(rotationAngle), 0.0f},
				{0.0f, 0.0f, 0.0f, 1.0f}
			};

			glm::vec3 rotatedVector(rotationAboutYAxis * translatedCameraCenterHomogeneous);
			glm::vec3 rotatedVectorTranslatedBackToCameraPosition = rotatedVector - a3.camera().direction();
			a3.camera().setCenter(rotatedVectorTranslatedBackToCameraPosition);
		}

		if (dragVector[1] != 0.0f)
		{
			glm::vec4 translatedCameraCenterHomogeneous(a3.camera().center() + a3.camera().direction(), 1.0f);

			float rotationAngle = -dragVector[1] * PI / 2;
			if (a3.camera().direction()[2] > 0.0f)
			{
				rotationAngle *= -1;
			}
			glm::mat4 rotationAboutXAxis = {
				{1.0f, 0.0f, 0.0f, 0.0f},
				{0.0f, cos(rotationAngle), -sin(rotationAngle), 0.0f},
				{0.0f, sin(rotationAngle), cos(rotationAngle), 0.0f},
				{0.0f, 0.0f, 0.0f, 1.0f}
			};

			glm::vec3 rotatedVector(rotationAboutXAxis * translatedCameraCenterHomogeneous);
			glm::vec3 rotatedVectorTranslatedBackToCameraPosition = rotatedVector - a3.camera().direction();
			a3.camera().setCenter(rotatedVectorTranslatedBackToCameraPosition);
		}
		a3.updateDragStart();
	}
}
void dragSelectedPoint(Assignment3& a3, CPU_Geometry& square, GPU_Geometry& pointsGPUGeom, GPU_Geometry& linesGPUGeom)
{
	if (a3.mouseDragging())
	{
		glm::vec3& selectedPoint = square.verts.at(a3.selectedIndex());
		if (abs(a3.convertedXPos() - selectedPoint[0]) > collisionThreshold)
		{
			selectedPoint[0] = a3.convertedXPos();
		}
		if (abs(a3.convertedYPos() - selectedPoint[1]) > collisionThreshold)
		{
			selectedPoint[1] = a3.convertedYPos();
		}

		// Reset the points colors to red and selected to blue
		a3.updatePoints();

		// Reset the colors to green
		a3.updateLines();
	}
}
void deCasteljauBezierGenerator(CPU_Geometry controlPointsGeom, CPU_Geometry& bezierCurve, GPU_Geometry& bezierGPUGeom)
{
	if (controlPointsGeom.verts.size() >= 2)
	{
		bezierCurve.verts.clear();
		bezierCurve.cols.clear();

		int degree = controlPointsGeom.verts.size() - 1;
		std::vector<glm::vec3> intermediatePoints = controlPointsGeom.verts;


		for (float u = 0; u < 1.0f; u += 1.0f / (float)numPointsOnGeneratedCurve)
		{
			for (int i = 0; i < degree; i++)
			{
				for (int j = 0; j < degree - i; j++)
				{
					intermediatePoints[j] = (1-u)* intermediatePoints[j] + u* intermediatePoints[j+1];
				}
			}

			bezierCurve.verts.push_back(intermediatePoints[0]);
			bezierCurve.cols.push_back(generatedCurveColour);
		}

		updateGPUGeometry(bezierGPUGeom, bezierCurve);
	}
}
void bsplineGenerator(CPU_Geometry controlPointsGeom, CPU_Geometry& bsplineCurve, GPU_Geometry& bsplineGPUGeom)
{
	if (controlPointsGeom.verts.size() >= 2)
	{
		std::vector<glm::vec3> controlPoints = controlPointsGeom.verts;

		int numIterations = log2(numPointsOnGeneratedCurve / controlPoints.size());

		for(int iteration = 0; iteration < numIterations; iteration++)
		{
			bsplineCurve.verts.clear();

			for (int i = 0; i < controlPoints.size() - 1; i++)
			{
				bsplineCurve.verts.push_back((3.0f / 4.0f) * controlPoints[i] + (1.0f / 4.0f) * controlPoints[i + 1]);
				bsplineCurve.verts.push_back((1.0f / 4.0f) * controlPoints[i] + (3.0f / 4.0f) * controlPoints[i + 1]);
			}

			//Modify beginning and end of curve to make it an open curve
			bsplineCurve.verts[0] = controlPoints[0];
			bsplineCurve.verts[1] = (1.0f/2.0f)*controlPoints[0] + (1.0f/2.0f)*controlPoints[1];

			bsplineCurve.verts[bsplineCurve.verts.size() - 2] = (1.0f/2.0f)*controlPoints[controlPoints.size() - 1] + (1.0f / 2.0f) * controlPoints[controlPoints.size() - 2];
			bsplineCurve.verts[bsplineCurve.verts.size() - 1] = controlPoints[controlPoints.size() - 1];

			controlPoints = bsplineCurve.verts;
		}

		//Colour b-spline curve
		bsplineCurve.cols.clear();
		for (int i = 0; i < bsplineCurve.verts.size(); i++)
		{
			bsplineCurve.cols.push_back(generatedCurveColour);
		}

		bsplineCurve_ = bsplineCurve;
		bsplineGeneratedForSurface = true;
		updateGPUGeometry(bsplineGPUGeom, bsplineCurve);
	}
}
void generateFloorGrid(CPU_Geometry& floorGrid, GPU_Geometry& floorGridGPUGeom)
{
	for (float i = -0.2f; i < 0.2f; i += floorGridStep)
	{
		for (float j = -1.0f; j < 1.0f; j += floorGridStep)
		{
			float firstRow = j;
			float secondRow = j + floorGridStep;
			float firstColumn = i;
			float secondColumn = i + floorGridStep;

			floorGrid.verts.push_back({ firstRow, 0.0f, firstColumn });
			floorGrid.verts.push_back({ secondRow, 0.0f, firstColumn });
			floorGrid.verts.push_back({ secondRow, 0.0f, secondColumn });
			floorGrid.verts.push_back({ firstRow, 0.0f, secondColumn });

			for (int vertNum = 0; vertNum < 4; vertNum++)
			{
				floorGrid.cols.push_back({ 0.0f, 0.0f, 0.0f });
			}
		}
	}
	updateGPUGeometry(floorGridGPUGeom, floorGrid);
}

void generateSurfaceOfRevolution(CPU_Geometry bsplineCurve, CPU_Geometry& generatedSurface, GPU_Geometry& generatedSurfaceGPUGeom)
{
	generatedSurface.verts.clear();
	int numPointsOnSurfaceCurves = bsplineCurve.verts.size() - 1;
	float vIncrement = (2.0f * PI / (float)numPointsOnSurfaceCurves);
	for (int u = 0; u < numPointsOnSurfaceCurves; u++)
	{
		for (float v = 0; v < 2 * PI; v += vIncrement)
		{
			float surfaceXAtUV = bsplineCurve.verts[u][0] * cos(v);
			float surfaceYAtUV = bsplineCurve.verts[u][1];
			float surfaceZAtUV = bsplineCurve.verts[u][0] * sin(v);
			glm::vec3 surfaceAtUV = { surfaceXAtUV , surfaceYAtUV , surfaceZAtUV };

			float surfaceXAtUPlusOneAndV = bsplineCurve.verts[(u+1)][0] * cos(v);
			float surfaceYAtUPlusOneAndV = bsplineCurve.verts[(u+1)][1];
			float surfaceZAtUPlusOneAndV = bsplineCurve.verts[(u+1)][0] * sin(v);
			glm::vec3 surfaceAtUPlusOneAndV  = { surfaceXAtUPlusOneAndV , surfaceYAtUPlusOneAndV , surfaceZAtUPlusOneAndV };

			float surfaceXAtUAndVPlusOne = bsplineCurve.verts[(u + 1)][0] * cos(v + vIncrement);
			float surfaceYAtUAndVPlusOne = bsplineCurve.verts[(u + 1)][1];
			float surfaceZAtUAndVPlusOne = bsplineCurve.verts[(u + 1)][0] * sin(v + vIncrement);
			glm::vec3 surfaceAtUAndVPlusOne = { surfaceXAtUAndVPlusOne , surfaceYAtUAndVPlusOne , surfaceZAtUAndVPlusOne };

			float surfaceXAtUAndVBothPlusOne = bsplineCurve.verts[u][0] * cos(v + vIncrement);
			float surfaceYAtUAndVBothPlusOne = bsplineCurve.verts[u][1];
			float surfaceZAtUAndVBothPlusOne = bsplineCurve.verts[u][0] * sin(v + vIncrement);
			glm::vec3 surfaceAtUAndVBothPlusOne = { surfaceXAtUAndVBothPlusOne , surfaceYAtUAndVBothPlusOne , surfaceZAtUAndVBothPlusOne };
			
			generatedSurface.verts.push_back(surfaceAtUPlusOneAndV);
			generatedSurface.verts.push_back(surfaceAtUV);
			generatedSurface.verts.push_back(surfaceAtUAndVBothPlusOne);

			generatedSurface.verts.push_back(surfaceAtUV);
			generatedSurface.verts.push_back(surfaceAtUAndVPlusOne);
			generatedSurface.verts.push_back(surfaceAtUAndVBothPlusOne);

			generatedSurface.verts.push_back(surfaceAtUPlusOneAndV);
			generatedSurface.verts.push_back(surfaceAtUV);
			generatedSurface.verts.push_back(surfaceAtUAndVPlusOne);

			for (int vertexNum = 0; vertexNum < 9; vertexNum++)
			{
				generatedSurface.cols.push_back(generatedCurveColour);
			}
		}
	}

	updateGPUGeometry(generatedSurfaceGPUGeom, generatedSurface);
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired
	CursorPositionConverter converter(window);
	Camera camera(window, initialEye, initialCenter, initialUpVector);

	GLDebug::enable();

	CPU_Geometry square;
	CPU_Geometry generatedCurve;
	CPU_Geometry generatedSurface;
	CPU_Geometry floorGrid;

	GPU_Geometry pointsGPUGeom;
	GPU_Geometry linesGPUGeom;
	GPU_Geometry generatedGPUGeom;
	GPU_Geometry generatedSurfaceGPUGeom;
	GPU_Geometry floorGridGPUGeom;

	glm::mat4 projectionMatrix = camera.perspectiveMatrix();
	glm::mat4 view = camera.viewMatrix();
	glm::vec3* selectedPoint = new glm::vec3(0.0f, 0.0f, 0.0f);

	generateFloorGrid(floorGrid, floorGridGPUGeom);

	//Initialize Empty Generated Curve
	generatedCurve.verts.resize(numPointsOnGeneratedCurve, glm::vec3{ 0.0f, 0.0f, 0.0f });
	generatedCurve.cols.resize(numPointsOnGeneratedCurve, glm::vec3{ 0.3f, 0.7f, 0.9f });
	updateGPUGeometry(generatedGPUGeom, generatedCurve);

	// CALLBACKS
	auto a3 = std::make_shared<Assignment3>(square, generatedCurve, generatedGPUGeom, pointsGPUGeom, linesGPUGeom, converter, selectedPoint, camera);
	window.setCallbacks(a3);


	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");
	glPointSize(10.0f);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glEnable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (sceneNumber == 0)
		{
			dragSelectedPoint(*a3, square, pointsGPUGeom, linesGPUGeom);
		}
		else
		{
			dragCamera(*a3);
		}

		if (sceneNumber < 2)
		{
			if (isBezierCurve)
			{
				deCasteljauBezierGenerator(square, generatedCurve, generatedGPUGeom);
			}
			else
			{
				bsplineGenerator(square, generatedCurve, generatedGPUGeom);
			}
		}

		shader.use();
		projectionMatrix = camera.perspectiveMatrix();
		view = camera.viewMatrix();
		
		GLint viewMatrixShaderVariable = glGetUniformLocation(shader.programId(), "viewMatrix");
		glUniformMatrix4fv(viewMatrixShaderVariable, 1, GL_FALSE, &view[0][0]);

		GLint projectionMatrixShaderVariable = glGetUniformLocation(shader.programId(), "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixShaderVariable, 1, GL_FALSE, &projectionMatrix[0][0]);

		if (sceneNumber == 0)
		{
			//2D Curve Editor
			if (showControlPolygon)
			{
				linesGPUGeom.bind();
				glDrawArrays(GL_LINE_STRIP, 0, GLsizei(square.verts.size()));
			}

			if (showControlPoints)
			{
				pointsGPUGeom.bind();
				glDrawArrays(GL_POINTS, 0, GLsizei(square.verts.size()));
			}

			generatedGPUGeom.bind();
			glDrawArrays(GL_LINE_STRIP, 0, GLsizei(generatedCurve.verts.size()));
			bsplineGeneratedForSurface = false;
		}
		else if (sceneNumber == 1)
		{
			//3D Curve Viewer
			if (showControlPolygon)
			{
				linesGPUGeom.bind();
				glDrawArrays(GL_LINE_STRIP, 0, GLsizei(square.verts.size()));
			}

			if (showControlPoints)
			{
				pointsGPUGeom.bind();
				glDrawArrays(GL_POINTS, 0, GLsizei(square.verts.size()));
			}

			if (showFloorGrid)
			{
				floorGridGPUGeom.bind();
				for (int i = 0; i < floorGrid.verts.size(); i += 4)
				{
					glDrawArrays(GL_LINE_LOOP, i, 4);
				}
			}

			generatedGPUGeom.bind();
			glDrawArrays(GL_LINE_STRIP, 0, GLsizei(generatedCurve.verts.size()));
			bsplineGeneratedForSurface = false;
		}
		else if (sceneNumber == 2)
		{
			//3D Surface of Revolution Viewer
			if (wireFrame)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			if (!bsplineGeneratedForSurface)
			{
				bsplineGenerator(square, generatedCurve, generatedGPUGeom); //Use b-spline curve for surface of revolution
				bsplineGeneratedForSurface = true;
			}
			generateSurfaceOfRevolution(bsplineCurve_, generatedSurface, generatedSurfaceGPUGeom);

			generatedSurfaceGPUGeom.bind();
			glDrawArrays(GL_TRIANGLES, 0, GLsizei(generatedSurface.verts.size()));
		}

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		// Starting the new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Putting the text-containing window in the top-left of the screen.
		ImGui::SetNextWindowPos(ImVec2(5, 5));

		// Setting flags
		ImGuiWindowFlags textWindowFlags =
			ImGuiWindowFlags_NoMove |				// text "window" should not move
			ImGuiWindowFlags_NoResize |				// should not resize
			ImGuiWindowFlags_NoCollapse |			// should not collapse
			ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
			ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
			ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
			ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
			ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible

		// Begin a new window with these flags. (bool *)0 is the "default" value for its argument.
		ImGui::Begin("scoreText", (bool*)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f)));
		if (sceneNumber == 0)
		{
			ImGui::Text("Left click to add/select control points.");
			ImGui::Text("Left click and drag to move control points.");
			ImGui::Text("Press \"d\" to delete the selected (blue) control point.");
			ImGui::Text("Press \"r\" to reset the window and clear all control points.");
			ImGui::Text("Press \"UP KEY\" to toggle between bezier and b-spline curves.");
			ImGui::Text("Press \"DOWN KEY\" to toggle showing the control polygon.");
			ImGui::Text("Press \"RIGHT KEY\" to toggle showing the control points.");
			ImGui::Text("Press \"h\" to switch to the 3D view.");
			ImGui::Text("Press \"j\" to switch to the 3D surface of revolution scene view.");
		}
		else if(sceneNumber == 1)
		{
			ImGui::Text("Note: the camera is reset to the original starting position everytime\nyou change scenes.");
			ImGui::Text("Press \"g\" to switch to the 2D curve editor.");
			ImGui::Text("Press \"j\" to switch to the 3D surface of revolution scene view.");
			ImGui::Text("Press \"f\" to toggle showing the zx-plane grid.");
		}
		else if (sceneNumber == 2)
		{
			ImGui::Text("Note: the camera is reset to the original starting position everytime\nyou change scenes.");
			ImGui::Text("Press \"g\" to switch to the 2D curve editor.");
			ImGui::Text("Press \"h\" to switch to the 3D curve viewer.");
			ImGui::Text("Press \"t\" to toggle between wireframe and filled.");
		}

		if (sceneNumber < 2)
		{
			if (isBezierCurve)
			{
				ImGui::Text("Curve Type: Bezier");
			}
			else
			{
				ImGui::Text("Curve Type: B-Spline");
			}
		}
		ImGui::PopStyleColor();

		// End the window
		ImGui::End();

		ImGui::Render();	// Render the ImGui window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
