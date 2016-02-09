#ifndef PLAYER_H
#define PLAYER_H

#include "CharacterObject.h"
#include "InputManager.h"
#include "Camera.h"

class PlayerObject : public CharacterObject
{
public:
	PlayerObject();
	PlayerObject(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, int playerID, unsigned long netID = 0); //ok for now
	virtual ~PlayerObject();

	static void setStatics(InputManager * i, Camera * c) { inputManager = i; camera = c; }

	virtual int update(long elapsedTime);
	virtual int render(long totalElapsedTime);

	virtual string getTypeName() { return "PlayerObject"; }
	static PlayerObject * getPlayerPointer() { return playerPointer; }
protected:
	inline bool isKeyPressed(long e) { return inputManager->isKeyPressed(e); }
	inline bool isKeyReleased(long e) { return inputManager->isKeyReleased(e); }
	inline bool isKeyDown(long e) { return inputManager->isKeyDown(e); }
	inline bool isKeyUp(long e) { return inputManager->isKeyUp(e); }
	inline bool isMouseScrollUp() { return inputManager->isMouseScrollUp(); }
	inline bool isMouseScrollDown() { return inputManager->isMouseScrollDown(); }

private:
	static InputManager * inputManager;
	static Camera * camera;
	
	static PlayerObject * playerPointer;
};

#endif