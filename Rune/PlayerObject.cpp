#include "PlayerObject.h"

InputManager * PlayerObject::inputManager = NULL;
Camera * PlayerObject::camera = NULL;
PlayerObject * PlayerObject::playerPointer = NULL;

PlayerObject::PlayerObject(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, int playerID, unsigned long netID) : CharacterObject(m, t, s, position, direction, up, path, playerID, netID)
{
	playerPointer = this;
}
PlayerObject::~PlayerObject() 
{
	if (playerPointer == this) playerPointer = NULL;
}

int PlayerObject::update(long elapsedTime)
{
	if (isKeyDown(IM_W)) walkForward();
	if (isKeyDown(IM_S)) walkBackward();
	if (isKeyDown(IM_A)) walkLeft();
	if (isKeyDown(IM_D)) walkRight();
	//if (isKeyPressed(IM_SPACE)) jump();
	if (isKeyDown(IM_SPACE)) jump();

	if (isKeyPressed(IM_M1))	primaryClick();
	if (isKeyReleased(IM_M1))	primaryRelease();
	if (isKeyDown(IM_M1))		primaryDown();
	if (isKeyUp(IM_M1))			primaryUp();

	if (isKeyPressed(IM_M2))	secondaryClick();
	if (isKeyReleased(IM_M2))	secondaryRelease();
	if (isKeyDown(IM_M2))		secondaryDown();
	if (isKeyUp(IM_M2))			secondaryUp();

	if (isKeyPressed(IM_1))	changeSpell(0);
	if (isKeyPressed(IM_2))	changeSpell(1);
	if (isKeyPressed(IM_3))	changeSpell(2);
	if (isKeyPressed(IM_4))	changeSpell(3);
	if (isKeyPressed(IM_5))	changeSpell(4);

	if (getNetworkState() == NETWORK_STATE_OFFLINE) {
		if (isMouseScrollUp())
		{
			PropObject * temp = new PropObject("sword.obj", "error.tga", "default.glsl",
				camera->getPos() + camera->getLookAtVector(), camera->getLookAtVector(), camera->getUpVector(), "sword.prop");
			//RagdollObject * temp = new RagdollObject(box, t, s, camera->getPos(), camera->getLookAtVector(), camera->getUpVector());
			temp->setVelocity(camera->getLookAtVector()*10.0f);
			temp->setRotation(camera->getLookAtVector(), camera->getUpVector());
		}
		if (isMouseScrollDown())
		{
			PropObject * temp = new PropObject("box.obj", "error.tga", "default.glsl",
				camera->getPos() + camera->getLookAtVector(), camera->getLookAtVector(), camera->getUpVector(), "box1x1.prop");
			//RagdollObject * temp = new RagdollObject(box, t, s, camera->getPos(), camera->getLookAtVector(), camera->getUpVector());
			temp->setVelocity(camera->getLookAtVector()*10.0f);
			temp->setRotation(camera->getLookAtVector(), camera->getUpVector());
		}
	}

	POINT p = inputManager->getMouseMovement();
	turn(p.x*0.05f, p.y*0.05f);

	int ret = CharacterObject::update(elapsedTime);

	//setDirection(camera->getLookAtVector());
	camera->setPosition(btToGLM3(&getBTPosition()) + glm::vec3(0,1,0));
	camera->setLookAtVector(getDirection());
	//camera->update(elapsedTime);

	return ret;
}

int PlayerObject::render(long totalElapsedTime)
{
	return CharacterObject::render(totalElapsedTime);
}