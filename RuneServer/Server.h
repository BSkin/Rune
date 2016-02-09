#ifndef SERVER_H
#define SERVER_H

//#include "..\Rune\Client.h"
#include "..\Rune\Game.h"

#include "../Rune/glew.h"
#include "GLU.h" //<gl/GLU.h>
#include "GL.h" //<gl/GL.h>
#include "..\Rune\SDL2-2.0.3\include\SDL.h"
#include "..\Rune\SDL2-2.0.3\include\SDL_thread.h"
#include "..\Rune\SDL2-2.0.3\include\SDL_opengl.h"
#include <iostream>
#include <fcntl.h>
#include <io.h>
#include "HostListener.h"
#include <string>
#include <list>
using std::string;
using std::list;
using std::cout;
using std::cin;

/*#define DEBUG 1

#if DEBUG
	#include <vld.h>	
#endif */

class Server
{
public:
	Server(HWND hWnd);
	~Server();
	int run(const char * ini);
private:
	//Game Logic functions
	int update(long elapsedTime);
	int renderFrame();

	int synchClient(int clientID);
	/*int clearObjects();
	int addPropObject();
	int addMeshObject();
	int spawnPlayer(int playerIndex, glm::vec3 position, glm::vec3 lookAt);
	int killGameObject(int gameObjectIndex);
	int deleteGameObject(int gameObjectIndex);*/

	void loadConfig();
	void saveConfig();

	int loadResources();
	int loadLevel(string asdf);
	int initOpenGL();
	int initGLContexts();
	int initGlew();
	int initSDL();
	int initObjects();
	int clearObjects();
	int cleanupObjects();
	int initBullet(void);
	int cleanBullet(void);
	int killBullet(void);
	int applyCustomContacts();

	int getClientIDByAddress(unsigned long ipAddress, unsigned long port);

	int resizeWindow(int width, int height);

	HDC diplayWindow;
	HGLRC primaryContext, loadingThreadContext;
	SDL_Window * displayWindow;
	SDL_GLContext displayContext;

	GLuint base;
	unsigned long frameCount;
	static AssetManager * assetManager;
	//vector<GameObject *> * gameObjects;
	LinkedHashMap<unsigned long, GameObject *> * gameObjects;
	vector<CharacterObject *> * playerObjects;

	//Bullet Variables
	btAlignedObjectArray<btCollisionShape*>	collisionShapes;
	btDefaultCollisionConfiguration * collisionConfiguration;
	btCollisionDispatcher * dispatcher;
	btBroadphaseInterface * broadphase;
	btSequentialImpulseConstraintSolver * solver;
	btDiscreteDynamicsWorld * dynamicsWorld;
	static bool myContactProcessedCallback(btManifoldPoint& cp, void * body0, void * body1);
	void initTestCube();

	glm::mat4 view, projection;

	double renderRefreshRate, updateRate;

	void readServerData(string iniPath);

	int aquireClientsMutex(int index);
	int releaseClientsMutex(int index);

	HWND consoleWindow;
	bool running;
	int maxPlayers;
	
	int gameType;
	string mapName;
	list<string> mapRotation;

	vector<Client*> clients;
	Packet * serverEventsTCP;
	Packet * serverEventsUDP;
};

#endif