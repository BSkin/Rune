#define GLM_FORCE_RADIANS
#define _USE_MATH_DEFINES

#include <WinSock2.h>
#include <Windows.h>

#include "glew.h"
#include "GLU.h" //<gl/GLU.h>
#include "GL.h" //<gl/GL.h>
#include "SDL2-2.0.3\include\SDL.h"
#include "SDL2-2.0.3\include\SDL_thread.h"
#include "SDL2-2.0.3\include\SDL_opengl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"	
#include "btBulletDynamicsCommon.h"
#include "CollisionObject.h"

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include "InputManager.h"
#include "Camera.h"
#include "Utilities.h"
#include "Settings.h"
#include "AssetManager.h"
#include "Shader.h"
#include "Texture.h"
#include "Model.h"
#include "Effect.h"
#include "LinkedHashMap.h"
#include "GameObject.h"
#include "PhysicsObject.h"
#include "PropObject.h"
#include "MeshObject.h"
#include "HeightFieldObject.h"
#include "RagdollObject.h"
#include "PlayerObject.h"
#include "FullHealthKit.h"
#include "HalfHealthKit.h"
#include "Boat.h"
#include "Ocean.h"
#include "AI.h"
#include "ClientThread.h"
#include "NetworkFunctions.h"
#include "UDPManager.h"

#define DEBUG 0

#if DEBUG
	#include <vld.h>
#endif

#define FULLSCREEN	0
#define WINDOWED	1
#define BORDERLESS	2

using std::string;
using std::getline;
using std::stringstream;
using std::ostringstream;
using std::ifstream;
using std::ofstream;
using namespace Utilities;

class Game
{
public:
	Game(char* gameName);
	~Game(void);

	int init();
	int start();
	int cleanup();

	//static LRESULT CALLBACK globalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	int update(long elapsedTime, unsigned long frameCount);
	int renderFrame(long time);
	int render2D(long time);
	int loadLevel(string asdf);

	void loadConfig();
	void saveConfig();
	
	int setViewport(int width, int height);
	int resizeWindow(int width, int height);
	void startOrtho();
	void endOrtho();

	bool isConnectedToHost();

	GLvoid buildFont();
	GLvoid killFont();
	int drawText(int x, int y, const char * s);

	int initOpenGL();
	int initGlew();
	int initSDL();
	int initGLContexts();
	int initObjects();
	int clearObjects();
	int cleanupObjects();
	int killSDL();

	int initBullet(void);
	int cleanBullet(void);
	int killBullet(void);
	int applyCustomContacts();

	//Shortcut Functions
	inline bool isKeyDown(long e) { return inputManager.isKeyDown(e); }
	inline bool isKeyUp(long e) { return inputManager.isKeyUp(e); }
	inline bool isKeyPressed(long e) { return inputManager.isKeyPressed(e); }
	inline bool isKeyReleased(long e) { return inputManager.isKeyReleased(e); }
	#define activeShader Shader::getActiveShader()
	#define activeCamera Camera::getActiveCamera()

	bool activeWindow, running;
	short bits;
	int flags;
	char title[128];
	bool wireFrame;
	void toggleWireFrame();
	
	HDC diplayWindow;
	HGLRC primaryContext, loadingThreadContext;
	SDL_Window * displayWindow;
	SDL_GLContext displayContext;
	SDL_Thread * assetLoaderThread;
	static int startAssetLoader(void*);
	HANDLE clientThreadHandle;

	GLuint base;
	unsigned long frameCount;
	InputManager inputManager;
	static AssetManager * assetManager;
	list<Effect*> * effectList;
	LinkedHashMap<unsigned long, GameObject *> * gameObjects;
	//vector<GameObject *> * gameObjects;
	vector<CharacterObject *> * playerObjects;
	//vector<GameObject *> * deadObjects;
	vector<GameObject *> * oceanHeightModObjects;
	Texture * crosshairTexture;
	//PlayerObject * player;
	//Character * char1;
	//Boat * boat;
	//Ocean * ocean;

	bool connectedToHost;
	Client * client;
	Packet clientEventsTCP;
	Packet clientEventsUDP;
	void setUsername(string x);
	void changeUsername(string x);

	//Bullet Variables
	btAlignedObjectArray<btCollisionShape*>	collisionShapes;
	btDefaultCollisionConfiguration * collisionConfiguration;
	btCollisionDispatcher * dispatcher;
	btBroadphaseInterface * broadphase;
	btSequentialImpulseConstraintSolver * solver;
	btDiscreteDynamicsWorld * dynamicsWorld;
	//btGhostPairCallback * ghostPairCallback;
	static bool myContactProcessedCallback(btManifoldPoint& cp, void * body0, void * body1);
	//btIDebugDraw asdf;	<----------------------------TODO: Create an implementation which implements the drawLine method; 
	//														setDebugDrawer(asdf); dynamicsWorld->getDebugDrawer->setDebugMode(); dynamicsWorld->debugDrawWorld(); 
	void initTestCube();

	double frameRate;
	#define MAXFRAMESAMPLES 100
	int tickindex;
	double ticksum;
	double ticklist[MAXFRAMESAMPLES];

	double calcFps(double newtick);

	glm::mat4 view, projection;
	Camera camera;

	FrameBuffer * oceanHeightModBuffer;
	float heightModGridSize;

	int state;
	Model * quad;
	Texture * startScreen;
	Texture * errorTexture;
	Shader * defaultShader;
	Shader * textShader;
	Shader * quadShader;
	Shader * depthShader;

	//void updateBoatHeights();

	void draw3DRect(float x, float y, float z);
	void draw3DRect(float x, float y, float z, float rotation);
	void draw3DRect(glm::vec3 lookAt, glm::vec3 position, glm::vec3 up, glm::vec3 size);
	void draw2DRect(float x, float y, float width, float height, Texture * t);
	void draw2DRect(float x, float y, int width, int height, Texture * t);
	void draw2DRect(int x, int y, float width, float height, Texture * t);
	void draw2DRect(int x, int y, int width, int height, Texture * t);
};