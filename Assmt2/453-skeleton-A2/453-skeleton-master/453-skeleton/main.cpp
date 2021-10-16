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

//Constant Values
const glm::vec3 fireDiamondChildParentOffset = { 0.1f, 0.1f, 0.0f };
const glm::vec3 shipTrailingChildParentOffset = { 0.0f, -0.3f, 0.0f };
const float childScaleDownFactor = 2.0f;
const float aspectRatioXFactor = 0.09f;
const float aspectRatioYFactor = 0.06f;
const float defaultScalingFactor = 1.25;
const float translationLength = 0.01f;
const float collisionThreshold = 0.1f;
const int diamondsNeededToWin = 4;

//Global Variables used for Animation States and Game Logic
int numDiamondsCollected = 0;

//Ship Roation Animation
bool animatingARotation = false;
float animationIncrement = 0.0f;
const float numAnimationFrames = 60.0f; //The larger this number, the slower the animation appears

//Fire circling animation
float fireAnimationAngle = 0.0f;
const float fireAnimationNumFrames = 3000.0f; //The larger this number, the slower the animation appears
const float fireAnimationIncrement = 2 * PI / fireAnimationNumFrames;

//Diamonds rotating after winning animation
float diamondAnimationAngle = 0.0f;
const float diamondAnimationNumFrames = 2000.0f; //The larger this number, the slower the animation appears
const float diamondAnimationIncrement = 2 * PI / diamondAnimationNumFrames;

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
		offsetFromParent(0.0f, 0.0f, 0.0f),
		theta(0),
		previousTheta(0),
		scale(1),
		transformationMatrix(1.0f), // This constructor sets it as the identity matrix
		isCollidable(true)
	{
		scalingMatrix = {
			{scale * aspectRatioXFactor, 0.0f, 0.0f, 0.0f},
			{0.0f, scale * aspectRatioYFactor, 0.0f, 0.0f},
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

	void scaleObject(float scalingFactor = defaultScalingFactor, bool scaleUp = true)
	{
		if (!scaleUp)
		{
			scalingFactor = 1 / scalingFactor;
		}
		scale *= scalingFactor;
		offsetFromParent *= scalingFactor;
		updateScalingMatrix();
	}

	void updateTranslationMatrix()
	{
		translationMatrix = {
			{1.0f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{position[0] + offsetFromParent[0], position[1] + offsetFromParent[1], 0.0f, 1.0f}
		};

		for (GameObject* child : children)
		{
			child->updateTranslationMatrix();
		}
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
			{scale * aspectRatioXFactor, 0.0f, 0.0f, 0.0f},
			{0.0f, scale * aspectRatioYFactor, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}
		};
	}

	glm::mat4 updateTransformationMatrix()
	{
		transformationMatrix = translationMatrix * rotationMatrix * scalingMatrix;
		return transformationMatrix;
	}

	GameObject& addChild(GameObject* object, glm::vec3 childParentOffset)
	{
		GameObject* child = object;

		child->scaleObject(childScaleDownFactor, false);
		child->position = this->position;
		child->offsetFromParent = glm::vec3(rotationMatrix * glm::vec4(childParentOffset, 1.0f));
		child->updateTranslationMatrix();
		child->updateTransformationMatrix();

		children.push_back(child);
		return *child;
	}

	void removeChildren()
	{
		children.clear();
	}

	void setIsCollidable(bool)
	{
		isCollidable = false;
		for (GameObject* child : children)
		{
			child->isCollidable = false;
		}
	}

	std::vector<GameObject*> children;

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	glm::vec3 position;
	glm::vec3 offsetFromParent;
	float theta;
	float previousTheta;
	float scale;
	glm::mat4 transformationMatrix;
	glm::mat4 translationMatrix;
	glm::mat4 scalingMatrix;
	glm::mat4 rotationMatrix;
	bool isCollidable;
};

void rotatePlayerToOriginalPosition(GameObject& ship)
{
	ship.theta = 0;
	ship.updateRotationMatrixToDefault();
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

void translateObject(GameObject& object, float xIncrement, float yIncrement)
{
	glm::vec3 translation = { translationLength * xIncrement, translationLength * yIncrement, 0.0f };
	object.position = object.position + translation;
	object.updateTranslationMatrix();

	for (GameObject* child : object.children)
	{
		child->position = object.position;
	}
}

void createAndAddDiamondsToGameObjectsList(std::vector<GameObject*>& diamonds, std::vector<GameObject*>& fires)
{
	GameObject* diamond1 = new GameObject("textures/diamond.png", GL_NEAREST, 0.5f, 0.5f);
	GameObject* diamond2 = new GameObject("textures/diamond.png", GL_NEAREST, -0.5f, 0.5f);
	GameObject* diamond3 = new GameObject("textures/diamond.png", GL_NEAREST, 0.5f, -0.5f);
	GameObject* diamond4 = new GameObject("textures/diamond.png", GL_NEAREST, -0.5f, -0.5f);

	GameObject* fire1 = new GameObject("textures/fire.png", GL_NEAREST, 0.0f, 0.0f);
	GameObject* fire2 = new GameObject("textures/fire.png", GL_NEAREST, 0.0f, 0.0f);
	GameObject* fire3 = new GameObject("textures/fire.png", GL_NEAREST, 0.0f, 0.0f);
	GameObject* fire4 = new GameObject("textures/fire.png", GL_NEAREST, 0.0f, 0.0f);

	diamond1->addChild(fire1, fireDiamondChildParentOffset);
	diamond2->addChild(fire2, fireDiamondChildParentOffset);
	diamond3->addChild(fire3, fireDiamondChildParentOffset);
	diamond4->addChild(fire4, fireDiamondChildParentOffset);

	diamonds.push_back(diamond1);
	diamonds.push_back(diamond2);
	diamonds.push_back(diamond3);
	diamonds.push_back(diamond4);

	fires.push_back(fire1);
	fires.push_back(fire2);
	fires.push_back(fire3);
	fires.push_back(fire4);
}

void resetScene(GameObject& ship, std::vector<GameObject*>& diamonds, std::vector<GameObject*>& fires)
{
	translateObject(ship, -100*ship.position[0], -100*ship.position[1]);
	for (int i = 0; i < numDiamondsCollected; i++)
	{
		ship.scaleObject(defaultScalingFactor, false);
	}

	rotatePlayerToOriginalPosition(ship);
	numDiamondsCollected = 0;
	ship.removeChildren();
	diamonds.clear();
	fires.clear();
	createAndAddDiamondsToGameObjectsList(diamonds, fires);
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
	MyCallbacks(GameObject& ship, std::vector<GameObject*>& diamonds, std::vector<GameObject*>& fires, CursorPositionConverter& converter) : ship_(ship), diamonds_(diamonds), fires_(fires), converter_(converter), xPos_(0), yPos_(0) {}

	virtual void keyCallback(int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			translateObject(ship_, cos(ship_.theta + PI/2), sin(ship_.theta + PI / 2));
		}
		else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
		{
			translateObject(ship_, -cos(ship_.theta + PI / 2), -sin(ship_.theta + PI / 2));
		}
		else if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			resetScene(ship_, diamonds_, fires_);
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

			if (abs(clickAngleFromVertical - ship_.theta) > PI)
			{
				clickAngleFromVertical = -(2 * PI - clickAngleFromVertical);
			}

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
	std::vector<GameObject*>& diamonds_;
	std::vector<GameObject*>& fires_;
	CursorPositionConverter& converter_;
	double xPos_;
	double yPos_;
};

void rotateChild(GameObject* child, float rotationAngle, glm::mat4 rotationMatrix)
{
	float offsetLength = glm::length(child->offsetFromParent);
	child->offsetFromParent = { offsetLength * sin(rotationAngle), offsetLength * -cos(rotationAngle), 0.0f };
	child->rotationMatrix = rotationMatrix;
	child->updateTranslationMatrix();

	for (GameObject* childsChild : child->children)
	{
		rotateChild(childsChild, rotationAngle, rotationMatrix);
	}
}

void handleDiamondCollision(GameObject& ship, GameObject* diamond)
{
	numDiamondsCollected++;
	diamond->setIsCollidable(false);
	ship.scaleObject();
	GameObject& child = ship.addChild(diamond, (float)numDiamondsCollected * shipTrailingChildParentOffset);
	rotateChild(&child, ship.theta, ship.rotationMatrix);
}

void checkForDiamondCollions(GameObject& ship, std::vector<GameObject*> diamonds)
{
	for (GameObject* diamond : diamonds)
	{
		float distanceBetweenShipAndDiamond = glm::distance(ship.position, diamond->position);
		if (diamond->isCollidable && distanceBetweenShipAndDiamond <= collisionThreshold)
		{
			handleDiamondCollision(ship, diamond);
		}
	}
}

void checkForFireCollions(GameObject& ship, std::vector<GameObject*>& diamonds, std::vector<GameObject*>& fires)
{
	for (GameObject* diamond : diamonds)
	{
		//fire->position is actually the diamond position, adding the rotated offsetFromParent gives where the fire is actually drawn.
		GameObject* fire = diamond->children[0];
		float offsetLength = glm::length(fire->offsetFromParent);
		glm::vec3 effectiveFirePosition = { fire->position[0] + offsetLength * cos(fire->theta), fire->position[1] + offsetLength * sin(fire->theta), 0.0f };

		float distanceBetweenShipAndFire = glm::distance(ship.position, effectiveFirePosition);
		if (fire->isCollidable && distanceBetweenShipAndFire <= collisionThreshold)
		{
			resetScene(ship, diamonds, fires);
		}
	}
}

void animateDiamondRotations(GameObject* diamond)
{
	diamondAnimationAngle = diamondAnimationAngle + diamondAnimationIncrement;

	diamond->theta = diamondAnimationAngle;
	diamond->rotationMatrix = {
		{cos(diamondAnimationAngle), -sin(diamondAnimationAngle), 0.0f, 0.0f},
		{sin(diamondAnimationAngle), cos(diamondAnimationAngle), 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
	};

	if (diamondAnimationAngle == 2 * PI)
	{
		diamondAnimationAngle = 0;
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

		for (GameObject* child : ship.children)
		{
			rotateChild(child, rotationAngle, ship.rotationMatrix);
		}

		if (animationIncrement == numAnimationFrames)
		{
			ship.previousTheta = ship.theta;
			animationIncrement = 0;
			animatingARotation = false;
		}
	}
}

void animateFire(GameObject& diamond)
{
	fireAnimationAngle += fireAnimationIncrement;
	GameObject* fire = diamond.children[0];
	fire->theta = fireAnimationAngle;
	float offsetLength = glm::length(fire->offsetFromParent);
	glm::vec3 effectiveDiamondPosition = diamond.position + diamond.offsetFromParent;

	fire->translationMatrix = {
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{effectiveDiamondPosition[0] + offsetLength*cos(fireAnimationAngle), effectiveDiamondPosition[1] + offsetLength * sin(fireAnimationAngle), 0.0f, 1.0f}
	};

	if (fireAnimationAngle == 2 * PI)
	{
		fireAnimationAngle = 0;
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
	std::vector<GameObject*> diamonds;
	std::vector<GameObject*> fires;
	createAndAddDiamondsToGameObjectsList(diamonds, fires);

	// CALLBACKS
	CursorPositionConverter positionConverter(window);
	window.setCallbacks(std::make_shared<MyCallbacks>(ship, diamonds, fires, positionConverter)); // can also update callbacks to new ones

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		checkForDiamondCollions(ship, diamonds);
		checkForFireCollions(ship, diamonds, fires);

		shader.use();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		animateShipRotation(ship);

		drawGameObject(ship, shader);
		for (GameObject* diamond : diamonds)
		{
			if (numDiamondsCollected == diamondsNeededToWin)
			{
				animateDiamondRotations(diamond);
			}
			drawGameObject(*diamond, shader);

			GameObject* fire = diamond->children[0];
			animateFire(*diamond);
			drawGameObject(*fire, shader);
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
		ImGui::Text("Score: %d\n", numDiamondsCollected); // Second parameter gets passed into "%d"
		if (numDiamondsCollected == diamondsNeededToWin)
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

	//Cleanup Diamonds and Fires
	diamonds.clear();
	fires.clear();

	// ImGui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
