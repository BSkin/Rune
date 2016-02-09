#include "Game.h"

AssetManager * Game::assetManager = NULL;

void initConsole()
{
	/*AllocConsole();

	long lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
	int hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	FILE *fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );
	fclose(fp);*/

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
}

void deleteConsole()
{
	FreeConsole();
}

Game::Game(char* gameName)
{
	title[127] = '\0';
	if (title == NULL) this->title[0] = '\0';
	//else strncpy(this->title, gameName, 127);
	else strncpy_s(this->title, gameName, 127);
	frameRate = 0;
	activeWindow = false;
	running = true;
	bits = 32;
	flags = 0;
	title[128];
	base = 0;

	Settings::setUsername("Peasant");
	Settings::setMaxFPS(60);
	Settings::setWindowWidth(1280);
	Settings::setWindowHeight(720);
	Settings::setWindowState(1);
	Settings::setServerIPString("127.0.0.1");
	Settings::setPortTCP(5150);
	Settings::setPortUDP(8851);
	
	//#if DEBUG
		initConsole();
	//#endif

	displayWindow =	NULL;
	displayContext = NULL;
	assetLoaderThread = NULL;

	state = 0;

	tickindex = 0;
	ticksum = 0.0;
	for (int i = 0; i < MAXFRAMESAMPLES; i++) ticklist[i] = 0;
}

Game::~Game()
{
	#if DEBUG
		deleteConsole();
	#endif
}

void Game::loadConfig()
{
	if (!dirExists("Config")) {
		SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, false};
		CreateDirectory("Config", &sa);
	}

	FILE * file;
	char configName[] = "Config\\Settings.ini";
	//file = fopen(configName, "r");
	fopen_s(&file, configName, "r");
	if (file == NULL) {
		//fclose(file);
		return;
	} //no file found, just use defaults
	fclose(file);

	ifstream fileStream(configName);
	string s;
	string var, val;
	while (fileStream.peek() != -1)
	{
		s = var = val = "";
		getline(fileStream, s);
		bool eq = false;
		int i = 0;

		while (i < (int)s.size())
		{
			if (s[i] == '=')	eq = true;
			else if (eq)		val += s[i];
			else				var += s[i];
			i++;
		}

		if (var == "username")				Settings::setUsername(val);
		else if (var == "maxFps")			Settings::setMaxFPS(atoi(val.c_str()));
		else if (var == "windowWidth")		Settings::setWindowWidth(atoi(val.c_str()));
		else if (var == "windowHeight")		Settings::setWindowHeight(atoi(val.c_str()));
		else if (var == "windowState")		Settings::setWindowState(atoi(val.c_str()));
		else if (var == "ipAddress")		Settings::setServerIPString(val);
		else if (var == "serverPort")		Settings::setPortTCP(atoi(val.c_str()));
		else if (var == "clientPortUDP")	Settings::setPortUDP(atoi(val.c_str()));
	}
	
	fileStream.close();
}

void Game::saveConfig()
{
	ofstream output;
	char configName[] = "Config\\Settings.ini";
	output.open(configName);
	
	const unsigned char * ver = glGetString(GL_VERSION);
	string v = "";
	int i = 0;
	while (ver[i] != ' ') { v += ver[i]; i++; }

	float version = (float)atof(v.c_str());

	output
		<< "username=" << Settings::getUsername() << '\n'
		<< "maxFps=" << Settings::getMaxFPS() << '\n'
		<< "windowWidth=" << Settings::getWindowWidth() << '\n'
		<< "windowHeight=" << Settings::getWindowHeight() << '\n'
		<< "windowState=" << Settings::getWindowState() << '\n'
		<< "ipAddress=" << Settings::getServerIPString() << '\n'
		<< "serverPort=" << Settings::getPortTCP() << '\n'
		<< "clientPortUDP=" << Settings::getPortUDP();

	output.close();
}

bool Game::isConnectedToHost()
{
	if (client == NULL) return false;
	return client->isConnected();
}

GLvoid Game::buildFont()
{
	HFONT   font;                       // Windows Font ID
    HFONT   oldfont;                    // Used For Good House Keeping
 
    base = glGenLists(96);                  // Storage For 96 Characters ( NEW )

	font = CreateFont(  
		-24,						//Height Of Font ( NEW ) -indicates height instead of width
		0,							//Width Of Font
		0,							//Angle Of Escapement
		0,							//Orientation Angle
		FW_NORMAL,					//Font Weight
		FALSE,						//Italic
		FALSE,						//Underline
		FALSE,						//Strikeout
		OUT_TT_PRECIS,				// Output Precision
		ANSI_CHARSET,				//Character Set Identifier
		CLIP_DEFAULT_PRECIS,		//Clipping Precision
		ANTIALIASED_QUALITY,		//Output Quality
		FF_DONTCARE|DEFAULT_PITCH,	//Family And Pitch
		"Arial"); 				//Font Name

	HDC deviceContext = GetDC (NULL);

	oldfont = (HFONT)SelectObject(deviceContext, font);       // Selects The Font We Want
    wglUseFontBitmaps(deviceContext, 32, 96, base);           // Builds 96 Characters Starting At Character 32
    SelectObject(deviceContext, oldfont);             // Selects The Font We Want
    DeleteObject(font);                 // Delete The Font
}

GLvoid Game::killFont()
{
    glDeleteLists(base, 96);                // Delete All 96 Characters ( NEW )
}

int Game::drawText(int x, int y, const char * s)
{
	if (s == NULL) return -1;
	
	char        text[256];              // Holds Our String
	va_list     ap;                 // Pointer To List Of Arguments
	
	va_start(ap, s);                  // Parses The String For Variables
		//vsprintf(text, s, ap);                // And Converts Symbols To Actual Numbers
		vsprintf_s(text, s, ap);                // And Converts Symbols To Actual Numbers
	va_end(ap);                     // Results Are Stored In Text
	
	//Determine the end of the string and convert each occurrence of '\n' to '\0'.
	char *end = text + strlen (text);
	for (char *next = text; *next != '\0'; next++) {
		if (*next == '\n') *next = '\0';
	}

	glColor4f(0.95f, 0.95f, 0.8f, 1.0f);

	long yOffset = y;
	for (char *line = text; line < end; line += strlen (line) + 1) {
		glWindowPos2i(x, y);//(yOffset-=32));
		//glRasterPos2i (x, yOffset); yOffset -= 32;
		glPushAttrib(GL_LIST_BIT);              // Pushes The Display List Bits     ( NEW )
		glListBase(base - 32);                  // Sets The Base Character to 32    ( NEW )

		glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);  // Draws The Display List Text  ( NEW )
		glPopAttrib();     
	}

	return 0;
}

int Game::init()
{
	loadConfig();
	initSDL();
	initGlew();
	initBullet();
	initGLContexts();
	initObjects();
	frameCount = 0;

	return 0;
}

int Game::initSDL()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	Uint32 windowFlags;

	if (Settings::getWindowState() == FULLSCREEN)		windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN;
	else if (Settings::getWindowState() == WINDOWED)	windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
	else if (Settings::getWindowState() == BORDERLESS) windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS;

	displayWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Settings::getWindowWidth(), Settings::getWindowHeight(), windowFlags);
	
	displayContext = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, displayContext); 
	
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY); 

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, bits);
	
    initOpenGL();
    resizeWindow(Settings::getWindowWidth(), Settings::getWindowHeight());

	SDL_SetRelativeMouseMode(SDL_TRUE);
    
	return 0;
}

void Game::toggleWireFrame()
{
	wireFrame = !wireFrame;
	if (wireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int Game::initOpenGL()
{
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping ( NEW )
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);  // clockwise oriented polys are front faces
	glCullFace(GL_BACK); // cull out the inner polygons... Set Culling property appropriately

	glShadeModel( GL_SMOOTH );							// Enable smooth shading
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );				// Set the background black
	glClearDepth( 1.0f );								// Depth buffer setup
	glEnable( GL_DEPTH_TEST );							// Enables Depth Testing
	glDepthFunc( GL_LEQUAL );							// The Type Of Depth Test To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);			// filled polys
	wireFrame = false;

	buildFont();

	return 0;
}

int Game::initGLContexts()
{
	diplayWindow = wglGetCurrentDC();
	primaryContext = wglGetCurrentContext();
	loadingThreadContext = wglCreateContext(diplayWindow);
	wglMakeCurrent(NULL, NULL);
	BOOL error = wglShareLists(primaryContext, loadingThreadContext);
	wglMakeCurrent(diplayWindow, primaryContext);

	return 0;
}

int Game::initObjects()
{
	//oceanHeightModBuffer = new FrameBuffer(1024, 1024);
	//oceanHeightModBuffer->addDepthTexture();
	//heightModGridSize = 256.0f;

	assetManager = new AssetManager();
	assetManager->setStatics(&diplayWindow, &loadingThreadContext, &primaryContext);
	//assetLoaderThread = SDL_CreateThread(startAssetLoader, "AssetLoaderThread", (void*)NULL);
	inputManager.init(&frameCount);
	effectList = new list<Effect*>();
	gameObjects = new LinkedHashMap<unsigned long, GameObject*>();
	playerObjects = new vector<CharacterObject*>();
	playerObjects->reserve(20);
	for (int i = 0; i < 20; i++) {
		playerObjects->push_back(NULL);
	}

	Effect::setStatics(effectList);
	RenderObject::setStatics(&view, &projection, assetManager, &clientEventsTCP, &clientEventsUDP, dynamicsWorld);
	RenderObject::setNetworkState(NETWORK_STATE_OFFLINE);
	GameObject::setStatics(gameObjects);
	PhysicsObject::setStatics(&collisionShapes);
	PlayerObject::setStatics(&inputManager, &camera);
	RigidObject::populateCollisionLibrary();
	//Boat::setStatics(oceanHeightModObjects);
	client = new Client();
	ClientThread::setStatics(client);
	NetworkFunctions::setStatics(gameObjects, playerObjects);// , 20); //Change later;
	NetworkFunctions::setNetworkState(NETWORK_STATE_OFFLINE);
	UDPManager::init(Settings::getPortUDP());
	setUsername(Settings::getUsername());

	//Event::setStatics();
	
	camera.activate();

	return 0;
}

int Game::clearObjects()
{
	while (gameObjects->size() > 0) {
		delete gameObjects->backValue();
		gameObjects->popBack();
	}
	
	while (effectList->size() > 0) {
		delete effectList->back();
		effectList->pop_back();
	}

	for (int i = 0; i < playerObjects->size(); i++)
		playerObjects->at(i) = NULL;
	
	cleanBullet();

	GameObject::resetNextNetID();

	return 0;
}

int Game::cleanupObjects()
{
	clearObjects();

	delete gameObjects;
	delete effectList;
	delete playerObjects;
	//deadObjects->clear();
	//delete deadObjects;
	//delete oceanHeightModObjects;

	RigidObject::deleteCollisionLibrary();
	assetManager->shutdown();
	SDL_WaitThread(assetLoaderThread, NULL);
	delete assetManager;
	return 0;
}

int Game::initGlew()
{
	static bool alreadyExecuted = false;
	if (alreadyExecuted) return -1;
	alreadyExecuted = true;

	const bool loggingExtensions = true; //Set to true to have extensions logged...
	char *extensions = (char *) glGetString (GL_EXTENSIONS);

	//bool isARBVertexBufferObjectExtensionPresent = isExtensionSupported (extensions, "GL_ARB_vertex_buffer_object");
	//if (isARBVertexBufferObjectExtensionPresent) {
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC) wglGetProcAddress ("glBindBufferARB");
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) wglGetProcAddress ("glDeleteBuffersARB");
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC) wglGetProcAddress ("glGenBuffersARB");
		glIsBufferARB = (PFNGLISBUFFERARBPROC) wglGetProcAddress ("glIsBufferARB");
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC) wglGetProcAddress ("glBufferDataARB");
		glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC) wglGetProcAddress ("glBufferSubDataARB");
		glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC) wglGetProcAddress ("glGetBufferSubDataARB");
		glMapBufferARB = (PFNGLMAPBUFFERARBPROC) wglGetProcAddress ("glMapBufferARB");
		glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC) wglGetProcAddress ("glUnmapBufferARB");
		glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC) wglGetProcAddress ("glGetBufferParameterivARB");
		glGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC) wglGetProcAddress ("glGetBufferPointervARB");
	//}
	
	//bool isGL_ARB_shader_objectsExtensionPresent = isExtensionSupported (extensions, "GL_ARB_shader_objects");
	//if (isGL_ARB_shader_objectsExtensionPresent) {
		glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress ("glAttachObjectARB");
		glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress ("glCompileShaderARB");
		glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress ("glCreateProgramObjectARB");
		glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress ("glCreateShaderObjectARB");
		glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress ("glDeleteObjectARB");
		glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)wglGetProcAddress ("glDetachObjectARB");
		glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)wglGetProcAddress ("glGetActiveUniformARB");
		glGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)wglGetProcAddress ("glGetAttachedObjectsARB");
		glGetHandleARB = (PFNGLGETHANDLEARBPROC)wglGetProcAddress ("glGetHandleARB");
		glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress ("glGetInfoLogARB");
		glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)wglGetProcAddress ("glGetObjectParameterfvARB");
		glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress ("glGetObjectParameterivARB");
		glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)wglGetProcAddress ("glGetShaderSourceARB");
		glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress ("glGetUniformLocationARB");
		glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)wglGetProcAddress ("glGetUniformfvARB");
		glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)wglGetProcAddress ("glGetUniformivARB");
		glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress ("glLinkProgramARB");
		glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress ("glShaderSourceARB");
		glUniform1fARB = (PFNGLUNIFORM1FARBPROC)wglGetProcAddress ("glUniform1fARB");
		glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)wglGetProcAddress ("glUniform1fvARB");
		glUniform1iARB = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress ("glUniform1iARB");
		glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)wglGetProcAddress ("glUniform1ivARB");
		glUniform2fARB = (PFNGLUNIFORM2FARBPROC)wglGetProcAddress ("glUniform2fARB");
		glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)wglGetProcAddress ("glUniform2fvARB");
		glUniform2iARB = (PFNGLUNIFORM2IARBPROC)wglGetProcAddress ("glUniform2iARB");
		glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)wglGetProcAddress ("glUniform2ivARB");
		glUniform3fARB = (PFNGLUNIFORM3FARBPROC)wglGetProcAddress ("glUniform3fARB");
		glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)wglGetProcAddress ("glUniform3fvARB");
		glUniform3iARB = (PFNGLUNIFORM3IARBPROC)wglGetProcAddress ("glUniform3iARB");
		glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)wglGetProcAddress ("glUniform3ivARB");
		glUniform4fARB = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress ("glUniform4fARB");
		glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)wglGetProcAddress ("glUniform4fvARB");
		glUniform4iARB = (PFNGLUNIFORM4IARBPROC)wglGetProcAddress ("glUniform4iARB");
		glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)wglGetProcAddress ("glUniform4ivARB");
		glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)wglGetProcAddress ("glUniformMatrix2fvARB");
		glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)wglGetProcAddress ("glUniformMatrix3fvARB");
		glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)wglGetProcAddress ("glUniformMatrix4fvARB");
		glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress ("glUseProgramObjectARB");
		glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)wglGetProcAddress ("glValidateProgramARB");

		// Other Shader Stuff
		glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress ("glCompileShader");
		glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress ("glCreateProgram");
		glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress ("glCreateShader");
		glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress ("glDeleteProgram");
		glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress ("glDeleteShader");
		glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress ("glDetachShader");
		glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)wglGetProcAddress ("glGetAttachedShaders");
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress ("glGetUniformLocation");
		glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress ("glUniform1f");
		glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress ("glUniform2f");
		glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress ("glUniform3f");
		glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress ("glUniform4f");
		glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress ("glUniform1i");		
	//}
	glewInit();

	return 0;
}

void Game::initTestCube()
{
	//new PropObject("ballworld.obj", "floor.tga", "default.glsl",
	//	glm::vec3(0,0,0), glm::vec3(0,0,1), glm::vec3(1,0,0), "model");
}

bool Game::myContactProcessedCallback(btManifoldPoint& cp, void * body0, void * body1)
{
	if (((btRigidBody*)body0)->getOwner() != NULL)
		((RigidObject*)((btRigidBody*)body0)->getOwner())->collisionCallback((RigidObject*)((btRigidBody*)body1)->getOwner());
	if (((btRigidBody*)body1)->getOwner() != NULL)
		((RigidObject*)((btRigidBody*)body1)->getOwner())->collisionCallback((RigidObject*)((btRigidBody*)body0)->getOwner());
	return true;
}

int Game::initBullet()
{
	/// collision configuration contains default setup for memory , collision setup . Advanced users can create their own configuration .
	collisionConfiguration = new btDefaultCollisionConfiguration ();

	///use the default collision dispatcher . For parallel processing you can use a diffent dispatcher ( see Extras / BulletMultiThreaded )
	dispatcher = new btCollisionDispatcher ( collisionConfiguration );

	/// btDbvtBroadphase is a good general purpose broadphase . You can also try out btAxis3Sweep .
	broadphase = new btDbvtBroadphase ();
	//broadphase = new btAxisSweep3 ();

	///the default constraint solver . For parallel processing you can use a different solver ( see Extras / BulletMultiThreaded )
	solver = new btSequentialImpulseConstraintSolver;
	
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration );
	//ghostPairCallback = new btGhostPairCallback();
	//dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback);
	dynamicsWorld->setGravity(btVector3(0, -9.81f*4.0f,0));
	gContactProcessedCallback = (ContactProcessedCallback)myContactProcessedCallback;

	return 0;
}

int Game::cleanBullet()
{	
	dynamicsWorld->clearForces();

	for (int i = dynamicsWorld->getNumConstraints() -1; i >= 0; i--) {
		btTypedConstraint * cons = dynamicsWorld->getConstraint(i);
		dynamicsWorld->removeConstraint(cons);
	}
	
	for (int i = dynamicsWorld->getNumCollisionObjects() -1; i >= 0; i--) {
		btCollisionObject * obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody * body = btRigidBody::upcast(obj);
		if (body && body->getMotionState()) {
			delete body->getMotionState();
		}
		dynamicsWorld -> removeCollisionObject ( obj );
		delete obj;
	}
	
	for (int j = 0; j < collisionShapes.size(); j++) {
		btCollisionShape * shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	return 0;
}

int Game::killBullet()
{
	cleanBullet();

	//delete ghostPairCallback;
	delete dynamicsWorld;
	delete solver;
	delete dispatcher;
	delete collisionConfiguration;
	delete broadphase;

	collisionShapes.clear();

	return 0;
}

int Game::loadLevel(string input)
{
	clearObjects();

	//Boat::setOcean(NULL);

	//initTestCube();
	
	new MeshObject("testramp.obj", "rock_redDeepCuts_2k_d.tga", "default.glsl",
		glm::vec3(0, -10, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));

	new FullHealthKit(glm::vec3(100, -10, 0));

	//player = new PlayerObject("Models\\player.obj", "Textures\\error.tga", "Shaders\\default.glsl", 
	//	glm::vec3(0,100,0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), "sphere2.prop");
	
	/*
	#define islandSize 50
	float * temp = new float[islandSize * islandSize];
	for (int x = 0; x < islandSize; x++) {
		for (int z = 0; z < islandSize; z++) {
			temp[z * islandSize + x] = -2.0f + 0.1f * (islandSize/2.0f - glm::distance(glm::vec2(x, z), glm::vec2(islandSize / 2, islandSize / 2)));
		}
	}
	new HeightFieldObject("Textures\\error.tga", "Shaders\\default.glsl", glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), temp, islandSize, islandSize, 0.0f);
	*/
	/*
	//char1 = new AI("Models\\player.obj", "Textures\\error.tga", "Shaders\\default.glsl", 
	//	glm::vec3(0,10,0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), "sphere2");


	ocean = new Ocean();
	//gameObjects->remove(ocean);
	//delete ocean;
	
	Boat::setOcean(ocean);*/
	
	return 0;
}

int Game::start()
{
	double elapsedTime = 0.0;
	double timePerFrame = 1000.0 / Settings::getMaxFPS();
	long lastTime = 0;
	
	double targetSleep = 0.0;
	double sleepRemainder = 0.0;
	double startSleep = 0.0;
	double sleepTime = 0.0;
	
	assetManager->forceLoadModel("Models\\quad.obj");
	quad = assetManager->getModel("Models\\quad.obj");
	assetManager->forceLoadTexture("Textures\\pixelTest.tga");
	startScreen = assetManager->getTexture("Textures\\pixelTest.tga");
	assetManager->forceLoadShader("Shaders\\default.glsl");
	assetManager->forceLoadShader("Shaders\\text.glsl");
	assetManager->forceLoadShader("Shaders\\quad.glsl");
	assetManager->forceLoadShader("Shaders\\Beam.glsl");
	assetManager->forceLoadShader("Shaders\\depthShader.glsl");
	assetManager->forceLoadShader("Shaders\\Ocean Shaders\\h0.glsl");
	assetManager->forceLoadShader("Shaders\\Ocean Shaders\\ht.glsl");
	assetManager->forceLoadShader("Shaders\\Ocean Shaders\\choppy.glsl");
	assetManager->forceLoadShader("Shaders\\Ocean Shaders\\FFTx.glsl");
	assetManager->forceLoadShader("Shaders\\Ocean Shaders\\FFTy.glsl");
	assetManager->forceLoadShader("Shaders\\Ocean Shaders\\scale.glsl");
	assetManager->forceLoadShader("Shaders\\Ocean Shaders\\finalOceanHeight.glsl");
	assetManager->forceLoadShader("Shaders\\Ocean Shaders\\ocean.glsl");
	defaultShader = assetManager->getShader("Shaders\\default.glsl");
	defaultShader->activate();
	textShader = assetManager->getShader("Shaders\\text.glsl");
	quadShader = assetManager->getShader("Shaders\\quad.glsl");
	depthShader = assetManager->getShader("Shaders\\depthShader.glsl");

	/*assetManager->forceLoadModel("Models\\ballworld.obj");	
	assetManager->forceLoadModel("Models\\rock_reddeepcuts.obj");
	assetManager->forceLoadModel("Models\\rock_reddeepcuts_corner.obj");
	assetManager->forceLoadModel("Models\\rocket.obj");
	assetManager->forceLoadModel("Models\\rocketlauncher.obj");
	assetManager->forceLoadModel("Models\\rocketsmall.obj");
	assetManager->forceLoadModel("Models\\snowman.obj");
	*/

	assetManager->forceLoadModel("Models\\box.obj");
	assetManager->forceLoadModel("Models\\player.obj");
	assetManager->forceLoadModel("Models\\sphere.obj");
	assetManager->forceLoadModel("Models\\sword.obj");
	assetManager->forceLoadModel("Models\\test.obj");
	assetManager->forceLoadModel("Models\\rune.obj");

	assetManager->forceLoadModel("Models\\testramp.obj");
	assetManager->forceLoadModel("Models\\fireballlauncher.obj");
	assetManager->forceLoadModel("Models\\fireball.obj");
	assetManager->forceLoadModel("Models\\fireballsmall.obj");
	assetManager->forceLoadModel("Models\\Beam.obj");
	assetManager->forceLoadModel("Models\\error.obj");
	assetManager->forceLoadModel("Models\\ocean.obj");

	assetManager->forceLoadTexture("Textures\\ctest.tga");
	assetManager->forceLoadTexture("Textures\\error.tga");
	assetManager->forceLoadTexture("Textures\\ctest2.tga");
	assetManager->forceLoadTexture("Textures\\floor.tga");
	assetManager->forceLoadTexture("Textures\\rock_redDeepCuts_2k_d.tga");
	assetManager->forceLoadTexture("Textures\\snowman.tga");
	assetManager->forceLoadTexture("Textures\\starttemp.tga");
	assetManager->forceLoadTexture("Textures\\FireballLauncher.tga");

	assetManager->forceLoadTexture("Textures\\green.tga");
	assetManager->forceLoadTexture("Textures\\yellow.tga");
	assetManager->forceLoadTexture("Textures\\purple.tga");
	assetManager->forceLoadTexture("Textures\\orange.tga");
	assetManager->forceLoadTexture("Textures\\brown.tga");
	assetManager->forceLoadTexture("Textures\\blue.tga");
	assetManager->forceLoadTexture("Textures\\grey.tga");
	assetManager->forceLoadTexture("Textures\\red.tga");
	assetManager->forceLoadTexture("Textures\\white.tga");
	assetManager->forceLoadTexture("Textures\\black.tga");
	assetManager->forceLoadTexture("Textures\\pink.tga");
	
	assetManager->forceLoadTexture("Textures\\crossTest.tga");
	crosshairTexture = assetManager->getTexture("Textures\\crossTest.tga");
	errorTexture = assetManager->getTexture("Textures\\error.tga");

	while(running)
	{
		frameCount++;
		lastTime = SDL_GetTicks();

		inputManager.clearTempValues();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)	return 1;
			else if (e.type == SDL_MOUSEBUTTONDOWN) inputManager.mouseKeyDown(e.button.button);
			else if (e.type == SDL_MOUSEBUTTONUP)	inputManager.mouseKeyUp(e.button.button);
			else if (e.type == SDL_KEYDOWN)			inputManager.keyDown(e.key.keysym.sym);
			else if (e.type == SDL_KEYUP)			inputManager.keyUp(e.key.keysym.sym);
			else if (e.type == SDL_MOUSEMOTION)		inputManager.updateMouse(e.motion.xrel, e.motion.yrel, e.motion.x, e.motion.y); 
			else if (e.type == SDL_MOUSEWHEEL)		{ inputManager.updateScroll(e.wheel.y); }
			else {}
		}
		
		if (update((long)(elapsedTime+sleepTime), frameCount) == -1) break;
		renderFrame(SDL_GetTicks());
		
		double averageElapsedTime = calcFps(elapsedTime+sleepTime);
		frameRate = 1000.0/averageElapsedTime;

		//Framerate Limit Calculations
		elapsedTime = SDL_GetTicks() - lastTime;
		targetSleep = timePerFrame - elapsedTime + sleepRemainder;
		if (targetSleep > 0) sleepRemainder = targetSleep - (Uint32)targetSleep;

		startSleep = SDL_GetTicks();
		while ((Uint32)(startSleep+targetSleep) > SDL_GetTicks());
		sleepTime = SDL_GetTicks() - startSleep;
	}
    return 0;
}

int Game::startAssetLoader(void * data)
{
	assetManager->startAssetLoader();
	return 0;
}

int Game::cleanup()
{
	ClientThread::cleanup();
	WaitForSingleObject(clientThreadHandle, INFINITE);
	UDPManager::cleanup();
	delete client;
	WSACleanup();
	
	cleanupObjects();
	saveConfig();

	Settings::cleanup();
	defaultShader = 0;
	quad = 0;
	startScreen = 0;

	
	killBullet();
	killSDL();
	killFont();

	//#if DEBUG
	//	_CrtDumpMemoryLeaks();
	//#endif

	PostQuitMessage(0);
	return 0;
}

int Game::killSDL() 
{
	wglDeleteContext(loadingThreadContext);
	SDL_GL_DeleteContext(displayContext);
    SDL_DestroyWindow(displayWindow);
	SDL_Quit();

	return 0;
}

int Game::resizeWindow(int width, int height) {
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	if (width == desktop.right && height == desktop.bottom)
		width++;
	
	SDL_SetWindowSize(displayWindow, width, height); 

	//Setup a new viewport.
	glViewport (0, 0, width, height);
	Settings::setWindowWidth(width); 
	Settings::setWindowHeight(height);

	//Setup a new perspective matrix.
	GLdouble verticalFieldOfViewInDegrees = 40;
	GLdouble aspectRatio = height == 0 ? 1.0 : (GLdouble) width / (GLdouble) height;
	GLdouble nearDistance = 1.0;
	GLdouble farDistance = 2000.0;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (verticalFieldOfViewInDegrees, aspectRatio, nearDistance, farDistance);

	//Get back to default mode.
	glMatrixMode (GL_MODELVIEW);

	return 0;
}

double Game::calcFps(double newtick)
{
	ticksum-=ticklist[tickindex];  /* subtract value falling off */
    ticksum+=newtick;              /* add new value */
    ticklist[tickindex]=newtick;   /* save new value so it can be subtracted later */
    if(++tickindex==MAXFRAMESAMPLES)    /* inc buffer index */
        tickindex=0;

    /* return average */
    return((ticksum)/MAXFRAMESAMPLES);
}

void Game::startOrtho() {
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
		glLoadIdentity ();
		glOrtho (0.0, (GLfloat) Settings::getWindowWidth(), 0.0, (GLfloat) Settings::getWindowHeight(), -100.0f, 100.0f);
		glMatrixMode (GL_MODELVIEW);
		glPushMatrix ();
			glLoadIdentity ();
}

void Game::endOrtho() {
			glMatrixMode (GL_PROJECTION);
		glPopMatrix ();
		glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();
}

void Game::changeUsername(string x) //notifies the server
{
	Settings::setUsername(x);
	client->aquireMutex();
		client->setName(x);
		//push changeName() to sendBuffer
	client->releaseMutex();
}

void Game::setUsername(string x) //does not notify the server, only use on startup
{
	Settings::setUsername(x);
	client->aquireMutex();
		client->setName(x);
	client->releaseMutex();
}

int Game::update(long elapsedTime, unsigned long frameCount)
{
 	connectedToHost = isConnectedToHost(); //stored locally, only need to access the mutex once
	int networkState = connectedToHost ? NETWORK_STATE_CLIENT : NETWORK_STATE_OFFLINE;
	RenderObject::setNetworkState(networkState);
	NetworkFunctions::setNetworkState(networkState);

	if (connectedToHost) {
		clientEventsTCP.clearSendBuffer();
		clientEventsTCP.clearReceiveBuffer();
		clientEventsUDP.clearSendBuffer();
		clientEventsUDP.clearReceiveBuffer();

		client->aquireMutex();
			DataBuffer * temp = client->popReceiveData();
		client->releaseMutex();
		clientEventsTCP.appendToReceiveBuffer(temp);

		DataBuffer * udpBuffer = UDPManager::recvDatagram();
		while (udpBuffer != NULL) {
			clientEventsUDP.appendToReceiveBuffer(udpBuffer->getData() + 4, udpBuffer->getSize() - 4);
			delete udpBuffer;
			udpBuffer = UDPManager::recvDatagram();
		}
	}

	if (isKeyPressed(IM_ESCAPE)) {
		if (ClientThread::isRunning()) {
			ClientThread::disconnectFromServer();
			WaitForSingleObject(clientThreadHandle, INFINITE);
		}
		return -1;
	}

	if (isKeyPressed(IM_NUM7)) toggleWireFrame();

	if (state == 0)	{
		if (isKeyPressed(IM_SPACE)) {
			state = 1;
			loadLevel("ASDF");
		}
		return 0;
	}
		
	if (isKeyPressed(IM_O) && !connectedToHost) 
	{
		loadLevel("ASDF");
	}

	if (isKeyPressed(IM_P)) {
		if (!connectedToHost) {
			if (PlayerObject::getPlayerPointer() != NULL) {
				GameObject * tempObject = gameObjects->getValue(PlayerObject::getPlayerPointer()->getNetID());
				if (tempObject != NULL) {
					gameObjects->remove(PlayerObject::getPlayerPointer()->getNetID());
					delete tempObject;
				}

				/*vector<GameObject*>::iterator position = std::find(gameObjects->begin(), gameObjects->end(), PlayerObject::getPlayerPointer());
				if (position != gameObjects->end()) {
					delete *position;
					gameObjects->erase(position);
					//delete player;
					//player = NULL;
				}*/
			}
			new PlayerObject("player.obj", "error.tga", "default.glsl",
				camera.getPos(), camera.getLookAtVector(), camera.getUpVector(), "sphere2.prop", Settings::getClientID());
		}
		else clientEventsTCP.appendToSendBuffer(NetworkFunctions::createSpawnPlayerRequestBuffer(Settings::getClientID(), camera.getPos(), camera.getLookAtVector(), 0));
	}

	if (isKeyPressed(IM_J)) {
		for (list<pair<unsigned long, GameObject*>>::iterator iter = gameObjects->begin(); iter != gameObjects->end(); iter++)
			printf("%lu,", iter->second->getNetID());
		printf("\n");
	}

	if (isKeyPressed(IM_L)) {
		if (connectedToHost) {
			ClientThread::disconnectFromServer();
		}
		else {
			//char * buffer = new char[32]; memset(buffer, '\0', 32);
			//strcat(buffer, serverIPAddress.c_str());
			//strcat(buffer, ":\0");
			//char port[6]; memset(port, '\0', 6);
			//itoa(DEFAULT_PORT_TCP, port, 10);
			//strcat(buffer, port);
			clientThreadHandle = ClientThread::connectToServer(Settings::getServerIP(), Settings::getPortTCP());
		}
	}
	
	camera.update(elapsedTime);
	camera.turn(inputManager.getMouseMovement());
	if (isKeyDown(IM_W)) camera.moveForward(elapsedTime*0.1f);
	if (isKeyDown(IM_S)) camera.moveBack(elapsedTime*0.1f);
	if (isKeyDown(IM_A)) camera.moveLeft(elapsedTime*0.1f);
	if (isKeyDown(IM_D)) camera.moveRight(elapsedTime*0.1f);
	if (isKeyDown(IM_SPACE)) camera.moveUp(elapsedTime*0.1f);
	if (isKeyDown(IM_LCTRL)) camera.moveDown(elapsedTime*0.1f);
	
	dynamicsWorld->stepSimulation((float)elapsedTime*0.001f, 50, 1.0f/(float)Settings::getMaxFPS());

	list<pair<unsigned long, GameObject*>>::iterator objectsIter = gameObjects->begin();
	while (objectsIter != gameObjects->end())
	{
		objectsIter->second->update(elapsedTime);
		if (!connectedToHost && !objectsIter->second->isAlive()) {
			GameObject * tempObject = objectsIter->second;
			objectsIter = gameObjects->erase(objectsIter);
			delete tempObject;
		}
		else objectsIter++;
	}

	/*for (int i = 0; i < (int)gameObjects->size(); i++) {
		(*gameObjects)[i]->update(elapsedTime);
		if (!connectedToHost && !(*gameObjects)[i]->isAlive()) { //the host will tell us when to delete things
			delete (*gameObjects)[i];
			gameObjects->erase(gameObjects->begin() + i);
			i--;
		}
	}*/

	std::list<Effect*>::iterator effectsIter = effectList->begin();
	while (effectsIter != effectList->end())
	{
		(*effectsIter)->update(elapsedTime);
		if (!(*effectsIter)->isAlive()) {
			delete (*effectsIter);
			effectsIter = effectList->erase(effectsIter);
		}
		else effectsIter++;
	}

	if (connectedToHost) {
		NetworkFunctions::translateReceivedEvents(&clientEventsTCP);
		NetworkFunctions::translateReceivedEvents(&clientEventsUDP);

		int serverPortUDP;
		client->aquireMutex();
			client->pushSendData(clientEventsTCP.getSendBuffer(), clientEventsTCP.getSendBufferSize());
			serverPortUDP = client->getPortUDP();
		client->releaseMutex();

		int clientID = Settings::getClientID();
		if (clientID != -1 && playerObjects->at(Settings::getClientID()) != NULL) {
			if (PlayerObject::getPlayerPointer() != NULL) {
				clientEventsUDP.appendToSendBuffer(NetworkFunctions::createUpdateObjectBuffer(PlayerObject::getPlayerPointer()->getNetID()));
			}
			/*int playerIndex = -1;
			for (int i = 0; i < gameObjects->size(); i++) {
				if (gameObjects->at(i) == playerObjects->at(clientID)) {
					playerIndex = i;
					break;
				}
			}
			//player = (PlayerObject*)playerObjects->at(playerIndex);
			
			//Send Player Data over UDP
			clientEventsUDP.appendToSendBuffer(NetworkFunctions::createUpdateObjectBuffer(playerIndex));*/
		}
		//else player = NULL;

		UDPManager::sendDatagram(clientEventsUDP.getSendBuffer(), clientEventsUDP.getSendBufferSize(), Settings::getServerIP(), htons(serverPortUDP), frameCount);
	}

	return 0;
}

int Game::renderFrame(long totalElapsedTime) {
	FrameBuffer::setActiveFrameBuffer(0);
	glClearColor (0.4f, 0.6f, 1.0f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);

	if (state == 0)
	{
		draw2DRect(0.0f, 0.0f, 1.0f, 1.0f, startScreen);
	}
	else if (state == 1)
	{
		camera.calculateView(&view);
		camera.calculateProjection(&projection);

		/*if (isKeyDown(IM_SPACE)) {
			view = glm::lookAt(glm::vec3(256, -50, 256), glm::vec3(256, 50, 256), glm::vec3(0, 0, 1));
			projection = glm::ortho(-128.0f, 128.0f, -128.0f, 128.0f, 0.0f, 100.0f);
		}*/
		//for (list<GameObject*>::iterator it = gameObjects->begin(); it != gameObjects->end(); it++)
		//	(*it)->render(totalElapsedTime);
		/*for (int i = 0; i < (int)gameObjects->size(); i++)
			(*gameObjects)[i]->render(totalElapsedTime);*/

		for (std::list<pair<unsigned long, GameObject*>>::iterator objectsIter = gameObjects->begin(); objectsIter != gameObjects->end(); objectsIter++)
			objectsIter->second->render(totalElapsedTime);
		
		for (std::list<Effect*>::iterator effectsIter = effectList->begin(); effectsIter != effectList->end(); effectsIter++)
			(*effectsIter)->render(totalElapsedTime);

		render2D(totalElapsedTime);
	}
	
	SDL_GL_SwapWindow(displayWindow);
	return 0;
}

/*void Game::updateBoatHeights()
{
	FrameBuffer::setActiveFrameBuffer(oceanHeightModBuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	depthShader->activate();

	glm::vec3 camPos = activeCamera->getPos();
	view = glm::lookAt(glm::vec3(camPos.x, -50, camPos.z), glm::vec3(camPos.x, 50, camPos.z), glm::vec3(0, 0, 1));
	projection = glm::ortho(-heightModGridSize*0.5f, heightModGridSize*0.5f, -heightModGridSize*0.5f, heightModGridSize*0.5f, 0.0f, 100.0f);

	//for (list<GameObject*>::iterator it = oceanHeightModObjects->begin(); it != oceanHeightModObjects->end(); it++)
	//	(*it)->shaderlessRender(0);
	for (int i = 0; i < (int)oceanHeightModObjects->size(); i++)
		(*gameObjects)[i]->shaderlessRender(0);
}*/

void Game::draw3DRect(float x, float y, float z)
{
	glm::mat4 world;
	world = glm::translate(glm::vec3(x,y,z));
	glm::mat4 matrix = projection * view * world;
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(world));
	activeShader->setUniformMatrixf4("worldViewProj", matrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);

	quad->render();
}

void Game::draw3DRect(glm::vec3 direction, glm::vec3 position, glm::vec3 up, glm::vec3 size)
{
	glm::mat4 world, scale;
	world = glm::inverse((glm::lookAt(position, position+direction, up)));
	scale = glm::scale(size);
	glm::mat4 matrix = projection * view * world * scale;
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(world));
	activeShader->setUniformMatrixf4("worldViewProj", matrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);

	quad->render();
}

void Game::draw3DRect(float x, float y, float z, float rotation)
{
	glm::mat4 world;
	world = glm::translate(glm::vec3(x,y,z));
	world = glm::rotate(world, rotation, glm::vec3(0,1,0));
	glm::mat4 matrix = projection * view * world;
	glm::mat4 normalMatrix = glm::transpose(glm::inverse(world));
	activeShader->setUniformMatrixf4("worldViewProj", matrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);

	quad->render();
}

void Game::draw2DRect(float x, float y, float width, float height, Texture * t)
{
	Shader * prevShader = activeShader;
	quadShader->activate();
		
	glActiveTexture(GL_TEXTURE0);
	if (t != NULL) t->bindTexture();
	quadShader->setUniformTexture("textureSampler", 0);
	quadShader->setUniformf1("width", width*2.0f);
	quadShader->setUniformf1("height", height*2.0f);
	quadShader->setUniformf2("position", x*2.0f, y*2.0f);

	quad->render();

	if (prevShader != NULL) prevShader->activate();
}

void Game::draw2DRect(float x, float y, int width, int height, Texture * t) { 
	draw2DRect(x, y, (float)width/(float)Settings::getWindowWidth(), (float)height/(float)Settings::getWindowHeight(), t); 
}
void Game::draw2DRect(int x, int y, float width, float height, Texture * t) { 
	draw2DRect((float)x/(float)Settings::getWindowWidth(), (float)y/(float)Settings::getWindowHeight(), width, height, t); 
}
void Game::draw2DRect(int x, int y, int width, int height, Texture * t) { 
	draw2DRect((float)x/(float)Settings::getWindowWidth(), (float)y/(float)Settings::getWindowHeight(), (float)width/(float)Settings::getWindowWidth(), (float)height/(float)Settings::getWindowHeight(), t); 
}

int Game::render2D(long time)
{
	startOrtho();
		glDepthFunc(GL_ALWAYS);
			//draw2DRect(0, 0, 512, 512, ocean->getTestText());
			//draw2DRect(0, 0, 512, 512, oceanHeightModBuffer->getDepthTexture());
		
			textShader->activate();
			textShader->setUniformf3("textColour", 0.9f, 0.9f, 0.9f); 
			ostringstream s;
			s << "FPS:" << (int)(frameRate+0.5);
			drawText(10, Settings::getWindowHeight()-30, s.str().c_str());
			//drawText(Settings::getWindowWidth()/2-10, Settings::getWindowHeight()/2-10, "+");

			if (PlayerObject::getPlayerPointer() != NULL){
				s.str(string());
				s << "Airborne: " << (PlayerObject::getPlayerPointer()->getAir() ? "TRUE" : "False");
				drawText(210, Settings::getWindowHeight()-30, s.str().c_str());

				s.str(string());
				s << "Health: " << PlayerObject::getPlayerPointer()->getHealth() << "/" << PlayerObject::getPlayerPointer()->getMaxHealth();
				drawText(10, Settings::getWindowHeight() - 130, s.str().c_str());

				draw2DRect(0, 0, 64, 64, crosshairTexture);
			}

			/*
			const unsigned char * ver = glGetString(GL_VERSION);
			string v = "";
			int i = 0;
			while (ver[i] != ' ') { v += ver[i]; i++; }

			float version = atof(v.c_str());

			s.str(string());
			s << "OpenGL Version: " << version;
			drawText(210, Settings::getWindowHeight()-30, s.str().c_str());
			*/

			#if DEBUG //Debug text
				s.str(string());
				s << "Pos:";
				drawText(10, Settings::getWindowHeight()-60, s.str().c_str());
				s.str(string());
				s << camera.getPos().x;
				drawText(65, Settings::getWindowHeight()-60, s.str().c_str());
				s.str(string());
				s << camera.getPos().y;
				drawText(65, Settings::getWindowHeight()-85, s.str().c_str());
				s.str(string());
				s << camera.getPos().z;
				drawText(65, Settings::getWindowHeight()-110, s.str().c_str());

				s.str(string());
				s << "Camera Angle:";
				drawText(210, Settings::getWindowHeight()-60, s.str().c_str());
				s.str(string());
				s << camera.getAngle().x;
				drawText(285, Settings::getWindowHeight()-85, s.str().c_str());
				s.str(string());
				s << camera.getAngle().y;
				drawText(285, Settings::getWindowHeight()-110, s.str().c_str());
				s.str(string());
				s << camera.getAngle().z;
				drawText(285, Settings::getWindowHeight()-135, s.str().c_str());

				if (PlayerObject::getPlayerPointer() != NULL) {
					s.str(string());
					s << "Player Angle:";
					drawText(410, Settings::getWindowHeight() - 60, s.str().c_str());
					s.str(string());
					s << PlayerObject::getPlayerPointer()->getAngle().x;
					drawText(485, Settings::getWindowHeight() - 85, s.str().c_str());
					s.str(string());
					s << PlayerObject::getPlayerPointer()->getAngle().y;
					drawText(485, Settings::getWindowHeight() - 110, s.str().c_str());
					s.str(string());
				}
			#endif

		glDepthFunc(GL_LEQUAL);
	endOrtho();
	return 0;
}