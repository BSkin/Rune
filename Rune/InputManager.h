#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

//Tab, space, escape
#include "KeyDeclarations.h"

struct keyTimings { 
	unsigned long lastDown; 
	unsigned long lastUp; 
};

#include "SDL2-2.0.3\include\SDL.h"
#include "SDL2-2.0.3\include\SDL_opengl.h"

class InputManager
{
public:
	InputManager();
	~InputManager();
	void init(unsigned long * frameCount);
	void keyDown(long e);
	void keyUp(long e);
	void mouseKeyDown(long e);
	void mouseKeyUp(long e);

	void clearTempValues() 
		{ mousePosition.x = mousePosition.y = mouseMovement.x = mouseMovement.y = mouseScrollY = 0; }
	void updateScroll(Sint32 y) 
		{ mouseScrollY = y; }
	void updateMouse(Sint32 xmove, Sint32 ymove, Sint32 x, Sint32 y) 
		{ mouseMovement.x += xmove; mouseMovement.y += ymove; mousePosition.x = x; mousePosition.y = y; }

	bool isKeyDown(long e);
	bool isKeyUp(long e);
	bool isKeyPressed(long e);
	bool isKeyReleased(long e);
	POINT getMousePosition()	{ return mousePosition; }
	POINT getMouseMovement()	{ return mouseMovement; }
	bool isMouseScrollUp()		{ return mouseScrollY > 0; }
	bool isMouseScrollDown()	{ return mouseScrollY < 0; }
	Sint32 getMouseScroll()		{ return mouseScrollY; }
private:
	//z is 122
	long convertSDLKToIM(long e);
	keyTimings keys[HIGHEST_KEY+1];
	unsigned long * frameCount;
	POINT mousePosition, mouseMovement;
	Sint32 mouseScrollY;
};

#endif