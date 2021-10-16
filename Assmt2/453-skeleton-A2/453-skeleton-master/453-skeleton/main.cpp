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
glm::mat4 globalTransformationMatrix = {
	{1.0f, 0.0f, 0.0f, 0.0f},
	{0.0f, 1.0f, 0.0f, 0.0f},
	{0.0f, 0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 0.0f, 1.0f}
};

//State used to animate the rotations of the ship
bool animatingARotation = false;
float animationIncrement = 0.0f;
float numAnimationFrames = 60.0f;

CPU_Geometry gameObjectGeometry() {
	CPU_Geometry retGeom;

	// For full marks (Part IV), you'll need to use the following vertex coordinates instead.
	// Then, you'd get the correct scale/translation/rotation by passing in uniforms into
	// the vertex shader.

	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));

	// texture coordinates
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));
	return retGeom;
}

// An example struct for Game Objects.
// You are encouraged to customize this as you see fit.
struct GameObject {
	// Struct's constructor deals with the texture.
	// Also sets default position, theta, scale, and transformationMatrix
	GameObject(std::string texturePath, GLenum textureInterpolation, float xPos, float yPos) :
		texture(texturePath, textureInterpolation),
		position(xPos, yPos, 0.0f),
		theta(0),
		previousTheta(0),
		scale(1),
		transformationMatrix(1.0f), // This constructor sets it as the identity matrix
		isVisible(true)
	{
		scalingMatrix = {
			{scale * 0.09f, 0.0f, 0.0f, 0.0f},
			{0.0f, scale * 0.06f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}
		};

		translationMatrix = {
			{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{xPos, yPos, 0.0f, 1.0f}
		};

		rotationMatrix = {
			{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}
		};

		transformationMatrix = translationMatrix * rotationMatrix * scalingMatrix;

		cgeom = gameObjectGeometry();
		ggeom.setVerts(cgeom.verts);
		ggeom.setTexCoords(cgeom.texCoords);
	}

	void updateTranslationMatrix()
	{
		translationMatrix = {
			{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{position[0], position[1], 0.0f, 1.0f}
		};
	}

	void updateRotationMatrixToDefault()
	{
		rotationMatrix = {
			{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}
		};
	}

	void updateScalingMatrix()
	{
		scalingMatrix = {
			{scale * 0.09f, 0.0f, 0.0f, 0.0f},
			{0.0f, scale * 0.06f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}
		};
	}

	glm::mat4 updateTransformationMatrix()
	{
		transformationMatrix = translationMatrix * rotationMatrix * scalingMatrix;
		return transformationMatrix;
	}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	glm::vec3 position;
	float theta;
	float previousTheta;
	float scale;
	glm::mat4 transformationMatrix;
	glm::mat4 translationMatrix;
	glm::mat4 scalingMatrix;
	glm::mat4 rotationMatrix;
	bool isVisible;
};

void rotatePlayerToOriginalPosition(GameObject& ship)
{
	ship.theta = PI / 2;
	ship.updateRotationMatrixToDefault();
}

void scaleShip(GameObject& ship, bool scaleUp = true)
{
	float scalingFactor = 1.25;
	if (!scaleUp)
	{
		scalingFactor = 1 / scalingFactor;
	}
	ship.scale *= scalingFactor;
	ship.updateScalingMatrix();
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
	ship.updateTranslationMatrix();
}

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

float normalizedAngle(float angle)
{
	while (angle >= 2 * PI)
	{
		angle -= 2 * PI;
	}
	return angle;
}

class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(GameObject& ship, std::vector<GameObject*> gameObjects, CursorPositionConverter& converter) : ship_(ship), gameObjects_(gameObjects), converter_(converter), xPos_(0), yPos_(0) {}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			translateShip(ship_, cos(ship_.theta + PI/2), sin(ship_.theta + PI / 2));
		}
		else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			translateShip(ship_, -cos(ship_.theta + PI / 2), -sin(ship_.theta + PI / 2));
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
			float clickAngleFromVertical = clickAngleFromXAxis - PI/2;

			ship_.theta = clickAngleFromVertical;
			animatingARotation = true;
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

/*void rotatePlayer(GameObject& ship)
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
}*/

void checkForDiamondCollions(GameObject& ship, std::vector<GameObject*> diamonds)
{
	for (GameObject* diamond : diamonds)
	{
		float distanceBetweenShipAndDiamond = glm::distance(ship.position, diamond->position);
		if (diamond->isVisible)
		{
			if (distanceBetweenShipAndDiamond <= 0.1)
			{
				diamond->isVisible = false;
				scaleShip(ship);
			}
		}
	}
}

void animateShipRotation(GameObject& ship)
{
	if (animatingARotation)
	{
		animationIncrement++;
		float angleChange = ship.theta - ship.previousTheta;
		float angleIncrement = angleChange * (animationIncrement / numAnimationFrames);
		float rotationAngle = ship.previousTheta + angleIncrement;
		ship.rotationMatrix = {
			{cos(-rotationAngle), -sin(-rotationAngle), 0.0f, 0.0f},
			{sin(-rotationAngle), cos(-rotationAngle), 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}
		};

		if (animationIncrement == numAnimationFrames)
		{
			ship.previousTheta = ship.theta;
			animationIncrement = 0;
			animatingARotation = false;
		}
	}
}

void drawGameObject(GameObject& gameObject, ShaderProgram& shader)
{
	gameObject.ggeom.bind();
	gameObject.texture.bind();
	globalTransformationMatrix = gameObject.updateTransformationMatrix();
	GLint transformationMatrixShaderVariable = glGetUniformLocation(shader.programId(), "transformationMatrix");
	glUniformMatrix4fv(transformationMatrixShaderVariable, 1, GL_FALSE, &globalTransformationMatrix[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	gameObject.texture.unbind();
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
	GameObject ship("textures/ship.png", GL_NEAREST, 0.0f, 0.0f);
	GameObject diamond1("textures/diamond.png", GL_NEAREST, 0.5f, 0.5f);
	GameObject diamond2("textures/diamond.png", GL_NEAREST, -0.5f, 0.5f);
	GameObject diamond3("textures/diamond.png", GL_NEAREST, 0.5f, -0.5f);
	GameObject diamond4("textures/diamond.png", GL_NEAREST, -0.5f, -0.5f);
	
	std::vector<GameObject*> diamonds;
	diamonds.push_back(&diamond1);
	diamonds.push_back(&diamond2);
	diamonds.push_back(&diamond3);
	diamonds.push_back(&diamond4);

	// CALLBACKS
	CursorPositionConverter positionConverter(window);
	window.setCallbacks(std::make_shared<MyCallbacks>(ship, diamonds, positionConverter)); // can also update callbacks to new ones

	// RENDER LOOP
	while (!window.shouldClose()) {
		int score = 0;
		glfwPollEvents();

		checkForDiamondCollions(ship, diamonds);

		shader.use();
		//rotatePlayer(ship);

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		animateShipRotation(ship);

		drawGameObject(ship, shader);
		for (GameObject* diamond : diamonds)
		{
			//Only draw the diamond if it is marked visible
			if (diamond->isVisible)
			{
				drawGameObject(*diamond, shader);
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
