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

//Shader Uniforms
glm::vec3 translationVector = { 0.f, 0.f, 0.f };
glm::mat3 rotationMatrix = {
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 0.0f}
};

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
	bool isVisible;
};

void rotatePlayerToOriginalPosition(GameObject& ship)
{
	glm::mat3 rotationToXAxis = {
		{cos(ship.theta), -sin(ship.theta), 0.0f},
		{sin(ship.theta), cos(ship.theta), 0.0f},
		{0.f, 0.f, 0.f}
	};

	glm::mat3 rotationToOriginalPosition = {
		{cos(-PI / 2), -sin(-PI / 2), 0.0f},
		{sin(-PI / 2), cos(-PI / 2), 0.0f},
		{0.f, 0.f, 0.f}
	};

	std::vector<glm::vec3> newVerts;
	for (glm::vec3 vert : ship.cgeom.verts)
	{
		vert = vert - ship.position; //Translate to the origin
		vert = rotationToOriginalPosition * rotationToXAxis * vert; //Rotate about the origin
		vert = vert + ship.position; //Translate back to the ship position
		newVerts.push_back(vert);
	}

	ship.cgeom.verts = newVerts;
	ship.ggeom.setVerts(newVerts);
	ship.theta = PI / 2;
}

void scaleShip(GameObject& ship, bool scaleUp = true)
{
	float scalingFactor = 1.25;
	if (!scaleUp)
	{
		scalingFactor = 1 / scalingFactor;
	}
	std::vector<glm::vec3> newVerts;
	for (glm::vec3 vert : ship.cgeom.verts)
	{
		glm::vec3 newVert = vert - ship.position; //Translate to origin
		newVert = newVert * scalingFactor; //Scale
		newVert = newVert + ship.position; //Translate back to ship position
		newVerts.push_back(newVert);
	}

	ship.cgeom.verts = newVerts;
	ship.ggeom.setVerts(newVerts);

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
	float translationLength = 0.01f;
	glm::vec3 translation = { translationLength * xIncrement, translationLength * yIncrement, 0.0f };
	ship.position = ship.position + translation;
}

bool reset = false;

void resetScene(GameObject& ship, std::vector<GameObject*> gameObjects)
{
	translateShip(ship, -100*ship.position[0], -100*ship.position[1]);
	for (GameObject* gameObject : gameObjects)
	{
		if (!gameObject->isVisible)
		{
			scaleShip(ship, false);
			gameObject->isVisible = true;
		}
	}
	rotatePlayerToOriginalPosition(ship);
}

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(GameObject& ship, std::vector<GameObject*> gameObjects, CursorPositionConverter& converter) : ship_(ship), gameObjects_(gameObjects), converter_(converter), xPos_(0), yPos_(0) {}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			translateShip(ship_, cos(ship_.theta), sin(ship_.theta));
		}
		else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			translateShip(ship_, -cos(ship_.theta), -sin(ship_.theta));
		}
		else if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			resetScene(ship_, gameObjects_);
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
	std::vector<GameObject*> gameObjects_;
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
			vert = vert - ship.position; //Translate to the origin
			vert = rotationToIterationAngle * rotationToXAxis * vert; //Rotate about the origin
			vert = vert + ship.position; //Translate back to the ship position
			newVerts.push_back(vert);
		}

		ship.cgeom.verts = newVerts;
		ship.ggeom.setVerts(newVerts);
		ship.theta = currentIterationAngle;
		ship.numRotations--;
	}
}

void checkForDiamondCollions(GameObject& ship, std::vector<GameObject*> gameObjects)
{
	for (GameObject* gameObject : gameObjects)
	{
		float distanceBetweenShipAndObject = glm::distance(ship.position, gameObject->position);
		if (gameObject->isVisible)
		{
			if (distanceBetweenShipAndObject <= 0.1)
			{
				gameObject->isVisible = false;
				scaleShip(ship);
			}
		}
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

	GameObject diamond("textures/diamond.png", GL_NEAREST);
	glm::vec3 diamondPosition = { 0.5f, 0.5f, 0.0f };
	diamond.position = diamondPosition;
	diamond.cgeom = shipGeom(0.18f, 0.12f, diamondPosition[0], diamondPosition[1]);
	diamond.ggeom.setVerts(diamond.cgeom.verts);
	diamond.ggeom.setTexCoords(diamond.cgeom.texCoords);

	GameObject diamond2("textures/diamond.png", GL_NEAREST);
	glm::vec3 diamond2Position = {-0.5f, 0.5f, 0.0f };
	diamond2.position = diamond2Position;
	diamond2.cgeom = shipGeom(0.18f, 0.12f, diamond2Position[0], diamond2Position[1]);
	diamond2.ggeom.setVerts(diamond2.cgeom.verts);
	diamond2.ggeom.setTexCoords(diamond2.cgeom.texCoords);

	GameObject diamond3("textures/diamond.png", GL_NEAREST);
	glm::vec3 diamond3Position = { 0.5f, -0.5f, 0.0f };
	diamond3.position = diamond3Position;
	diamond3.cgeom = shipGeom(0.18f, 0.12f, diamond3Position[0], diamond3Position[1]);
	diamond3.ggeom.setVerts(diamond3.cgeom.verts);
	diamond3.ggeom.setTexCoords(diamond3.cgeom.texCoords);

	GameObject diamond4("textures/diamond.png", GL_NEAREST);
	glm::vec3 diamond4Position = { -0.5f, -0.5f, 0.0f };
	diamond4.position = diamond4Position;
	diamond4.cgeom = shipGeom(0.18f, 0.12f, diamond4Position[0], diamond4Position[1]);
	diamond4.ggeom.setVerts(diamond4.cgeom.verts);
	diamond4.ggeom.setTexCoords(diamond4.cgeom.texCoords);

	std::vector<GameObject*> gameObjects;
	gameObjects.push_back(&diamond);
	gameObjects.push_back(&diamond2);
	gameObjects.push_back(&diamond3);
	gameObjects.push_back(&diamond4);

	// CALLBACKS
	CursorPositionConverter positionConverter(window);
	window.setCallbacks(std::make_shared<MyCallbacks>(ship, gameObjects, positionConverter)); // can also update callbacks to new ones

	// RENDER LOOP
	while (!window.shouldClose()) {
		int score = 0;
		glfwPollEvents();

		checkForDiamondCollions(ship, gameObjects);

		shader.use();
		rotatePlayer(ship);
		ship.ggeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ship.texture.bind();

		translationVector = ship.position;
		rotationMatrix = {
			{cos(ship.theta), -sin(ship.theta), 0.0f},
			{sin(ship.theta), cos(ship.theta), 0.0f},
			{0.f, 0.f, 0.f}
		};
		GLint translationVectorShaderVariable = glGetUniformLocation(shader.programId(), "translationVector");
		glUniform3f(translationVectorShaderVariable, translationVector[0], translationVector[1], translationVector[2]);


		glDrawArrays(GL_TRIANGLES, 0, 6);
		ship.texture.unbind();
		translationVector = { 0.f, 0.f, 0.f };
		rotationMatrix = {
			{ 0.f, 0.f, 0.f },
			{ 0.f, 0.f, 0.f },
			{ 0.f, 0.f, 0.f }
		};
		glUniform3f(translationVectorShaderVariable, translationVector[0], translationVector[1], translationVector[2]);

		for (GameObject* gameObject : gameObjects)
		{
			if (gameObject->isVisible)
			{
				gameObject->ggeom.bind();
				gameObject->texture.bind();
				glDrawArrays(GL_TRIANGLES, 0, 6);
				gameObject->texture.unbind();
			}
			else
			{
				score++;
			}
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
		ImGui::Begin("scoreText", (bool *)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);
		ImGui::Text("Score: %d\n", score); // Second parameter gets passed into "%d"
		if (score == 4)
		{
			ImGui::SetWindowFontScale(3.0f);
			ImGui::Text("Congratulations!\nYou got all of the diamonds!\nYou won the game!");
		}

		// End the windowi
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
