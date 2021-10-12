#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#define PI 3.14159265359

// An example struct for Game Objects.
// You are encouraged to customize this as you see fit.
struct GameObject {
	// Struct's constructor deals with the texture.
	// Also sets default position, theta, scale, and transformationMatrix
	GameObject(std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation),
		position(0.0f, 0.0f, 0.0f),
		theta(PI/2),
		scale(1),
		transformationMatrix(1.0f), // This constructor sets it as the identity matrix
		rotationIncrement(0.0f),
		numRotations(0.0f)
	{}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	glm::vec3 position;
	float theta; // Object's rotation
	// Alternatively, you could represent rotation via a normalized heading vec:
	// glm::vec3 heading; //Will make sure this is always a unit vector
	float scale;
	glm::mat4 transformationMatrix;
	float rotationIncrement;
	float numRotations;
	float rotationFinishAngle;
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

float findAngleFromXAxisBasedOnQuadrants(glm::vec3 centreOfShipToClickedPosition, float arcTanClickAngle)
{
	float clickAngleFromXAxis = 0.0f;
	if (centreOfShipToClickedPosition[0] > 0 && centreOfShipToClickedPosition[1] > 0)
	{
		//Top Right Quadrant
		clickAngleFromXAxis = arcTanClickAngle;
	}
	else if (centreOfShipToClickedPosition[0] < 0 && centreOfShipToClickedPosition[1] > 0)
	{
		//Top Left Quadrant
		clickAngleFromXAxis = PI + arcTanClickAngle; //arcTanClickAngle is negative here
	}
	else if (centreOfShipToClickedPosition[0] > 0 && centreOfShipToClickedPosition[1] < 0)
	{
		//Bottom Right Quadrant
		clickAngleFromXAxis = 2 * PI + arcTanClickAngle; //arcTanClickAngle is negative here
	}
	else if (centreOfShipToClickedPosition[0] < 0 && centreOfShipToClickedPosition[1] < 0)
	{
		//Bottom Left Quadrant
		clickAngleFromXAxis = PI + arcTanClickAngle; //arcTanClickAngle is positive here
	}

	return clickAngleFromXAxis;
}

void translateShip(GameObject& ship, float xIncrement, float yIncrement)
{
	std::vector<glm::vec3> newVerts;
	float translationLength = 0.01f;
	glm::vec3 translationVector = { translationLength*xIncrement, translationLength*yIncrement, 0.0f };
	for (glm::vec3 vert : ship.cgeom.verts)
	{
		newVerts.push_back(vert + translationVector);
	}

	ship.cgeom.verts = newVerts;
	ship.ggeom.setVerts(newVerts);
}

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(GameObject& ship, CursorPositionConverter& converter) : ship_(ship), converter_(converter), xPos_(0), yPos_(0) {}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			float xIncrement = cos(ship_.theta);
			float yIncrement = sin(ship_.theta);
			translateShip(ship_, xIncrement, yIncrement);

		}
		else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			float xIncrement = -cos(ship_.theta);
			float yIncrement = -sin(ship_.theta);
			translateShip(ship_, xIncrement, yIncrement);
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
		{
			double convertedXPos = converter_.convertedXPos(xPos_);
			double convertedYPos = converter_.convertedYPos(yPos_);
		
			glm::vec3 centreOfShipToClickedPosition = { convertedXPos - ship_.position[0], convertedYPos - ship_.position[1], 0 };

			float arcTanClickAngle = atan(centreOfShipToClickedPosition[1] / centreOfShipToClickedPosition[0]);
			float clickAngleFromXAxis = findAngleFromXAxisBasedOnQuadrants(centreOfShipToClickedPosition, arcTanClickAngle);
			float angleDiff = clickAngleFromXAxis - ship_.theta;

			std::cout << "AngleDiff: " << angleDiff * 180 / PI << std::endl;

			ship_.numRotations = 150.f;
			if (abs(angleDiff) > PI)
			{
				clickAngleFromXAxis = clickAngleFromXAxis - (2 * PI);
				angleDiff = angleDiff - (2 * PI);
			}

			ship_.rotationFinishAngle = clickAngleFromXAxis;
			ship_.rotationIncrement = angleDiff / ship_.numRotations;
		}
	}

	virtual void cursorPosCallback(double xPos, double yPos) {
		xPos_ = xPos;
		yPos_ = yPos;
	}

private:
	GameObject& ship_;
	CursorPositionConverter& converter_;
	double xPos_;
	double yPos_;
};

CPU_Geometry shipGeom(float width, float height, float xOffset, float yOffset) {
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	CPU_Geometry retGeom;
	// vertices for the spaceship quad
	retGeom.verts.push_back(glm::vec3(-halfWidth + xOffset, halfHeight + yOffset, 0.f));
	retGeom.verts.push_back(glm::vec3(-halfWidth + xOffset, -halfHeight + yOffset, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth + xOffset, -halfHeight + yOffset, 0.f));
	retGeom.verts.push_back(glm::vec3(-halfWidth + xOffset, halfHeight + yOffset, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth + xOffset, -halfHeight + yOffset, 0.f));
	retGeom.verts.push_back(glm::vec3(halfWidth + xOffset, halfHeight + yOffset, 0.f));

	// For full marks (Part IV), you'll need to use the following vertex coordinates instead.
	// Then, you'd get the correct scale/translation/rotation by passing in uniforms into
	// the vertex shader.
	/*
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));
	*/

	// texture coordinates
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));
	return retGeom;
}

// END EXAMPLES
void rotatePlayer(GameObject& ship)
{
	if (ship.numRotations > 0)
	{
		glm::mat3 rotationToXAxis = {
			{cos(ship.theta), -sin(ship.theta), 0.0f},
			{sin(ship.theta), cos(ship.theta), 0.0f},
			{0.f, 0.f, 0.f}
		};

		float currentIterationAngle = ship.rotationFinishAngle - (ship.numRotations * ship.rotationIncrement);

		glm::mat3 rotationToIterationAngle = {
			{cos(currentIterationAngle), sin(currentIterationAngle), 0.0f},
			{sin(currentIterationAngle), -cos(currentIterationAngle), 0.0f},
			{0.f, 0.f, 0.f}
		};

		std::vector<glm::vec3> newVerts;
		for (glm::vec3 vert : ship.cgeom.verts)
		{
			newVerts.push_back(rotationToIterationAngle * rotationToXAxis * vert);
		}

		ship.cgeom.verts = newVerts;
		ship.ggeom.setVerts(newVerts);
		ship.theta = currentIterationAngle;
		ship.numRotations--;
	}
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// GL_NEAREST looks a bit better for low-res pixel art than GL_LINEAR.
	// But for most other cases, you'd want GL_LINEAR interpolation.
	GameObject ship("textures/ship.png", GL_NEAREST);
	ship.cgeom = shipGeom(0.18f, 0.12f, 0.f, 0.f);
	ship.ggeom.setVerts(ship.cgeom.verts);
	ship.ggeom.setTexCoords(ship.cgeom.texCoords);

	// CALLBACKS
	CursorPositionConverter positionConverter(window);
	window.setCallbacks(std::make_shared<MyCallbacks>(ship, positionConverter)); // can also update callbacks to new ones

	GameObject diamond("textures/diamond.png", GL_NEAREST);
	diamond.cgeom = shipGeom(0.18f, 0.12f, 0.5f, 0.5f);
	diamond.ggeom.setVerts(diamond.cgeom.verts);
	diamond.ggeom.setTexCoords(diamond.cgeom.texCoords);

	// RENDER LOOP
	while (!window.shouldClose()) {
		int score;
		glfwPollEvents();

		shader.use();
		rotatePlayer(ship);
		ship.ggeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ship.texture.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		ship.texture.unbind();

		diamond.ggeom.bind();
		diamond.texture.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		diamond.texture.unbind();
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
		ImGui::Begin("scoreText", (bool *)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);
		ImGui::Text("Score: %d", 0); // Second parameter gets passed into "%d"

		// End the window.
		ImGui::End();

		ImGui::Render();	// Render the ImGui window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

		window.swapBuffers();
	}
	// ImGui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
