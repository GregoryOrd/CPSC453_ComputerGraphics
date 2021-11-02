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

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

const glm::vec3 selectedColour = { 0.f, 0.f, 1.0f };
const glm::vec3 nonSelectedColour = { 1.f, 0.0f, 0.0f };
const float collisionThreshold = 0.01;

// We gave this code in one of the tutorials, so leaving it here too
void updateGPUGeometry(GPU_Geometry &gpuGeom, CPU_Geometry const &cpuGeom) {
	gpuGeom.bind();
	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);
}

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
	Assignment3(CPU_Geometry& square, GPU_Geometry& pointsGPUGeom, GPU_Geometry& linesGPUGeom, CursorPositionConverter& converter, glm::vec3* selectedPoint)
		: square_(square), pointsGPUGeom_(pointsGPUGeom), linesGPUGeom_(linesGPUGeom), converter_(converter), xPos_(0.f), yPos_(0.f), mouseDragging_(false), selectedIndex_(-1)
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

			selectedIndex_ = -1;

			updatePoints();
			updateLines();
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS && !mouseDragging_)
		{
			selectedIndex_ = -1;
			mouseDragging_ = true;
			colourPointsAndSetSelectedIndex();

			if (selectedIndex_ == -1)
			{
				addPoint();
			}

			updatePoints();
			updateLines();
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

private:
	CursorPositionConverter& converter_;
	CPU_Geometry& square_;
	GPU_Geometry& pointsGPUGeom_;
	GPU_Geometry& linesGPUGeom_;
	float xPos_;
	float yPos_;
	int selectedIndex_;
	bool mouseDragging_;
};

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

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired
	CursorPositionConverter converter(window);

	GLDebug::enable();

	CPU_Geometry square;
	GPU_Geometry pointsGPUGeom;
	GPU_Geometry linesGPUGeom;
	glm::vec3* selectedPoint = new glm::vec3(0.0f, 0.0f, 0.0f);

	bool isBezierCurve = true;

	// CALLBACKS
	auto a3 = std::make_shared<Assignment3>(square, pointsGPUGeom, linesGPUGeom, converter, selectedPoint);
	window.setCallbacks(a3);


	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");
	glPointSize(10.0f);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		dragSelectedPoint(*a3, square, pointsGPUGeom, linesGPUGeom);

		shader.use();

		linesGPUGeom.bind();
		glDrawArrays(GL_LINE_STRIP, 0, GLsizei(square.verts.size()));

		pointsGPUGeom.bind();
		glDrawArrays(GL_POINTS, 0, GLsizei(square.verts.size()));

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
		ImGui::Text("Left click to add/select control points.");
		ImGui::Text("Left click and drag to move control points.");
		ImGui::Text("Press \"d\" to delete the selected (blue) control point.");
		ImGui::Text("Press \"r\" to reset the window and clear all control points.");

		if (isBezierCurve)
		{
			ImGui::Text("Curve Type: Bezier");
		}
		else
		{
			ImGui::Text("Curve Type: B-Spline");
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
