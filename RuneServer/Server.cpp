#include "Server.h"

AssetManager * Server::assetManager = NULL;

Server::Server(HWND hWnd)
{
	consoleWindow = hWnd;
	gameType = 0;
	mapName = "test";
	mapRotation = list<string>();
	mapRotation.push_back("test");
	maxPlayers = 10;
	updateRate = 64.0;
	renderRefreshRate = 10.0f;
	frameCount = 0;

	Settings::setWindowWidth(800);
	Settings::setWindowHeight(600);
	Settings::setWindowState(1);
	Settings::setPortTCP(5150);
	Settings::setPortUDP(8850);
}

Server::~Server()
{

}

int Server::run(const char * ini)
{
	readServerData(ini);

	//clients = new Client[maxPlayers];
	clients = vector<Client*>();
	clients.reserve(maxPlayers);
	for (int i = 0; i < maxPlayers; i++) {
		clients.push_back(new Client());
		clients[i]->setID(i);
	}
		
	double elapsedTime = 0.0;
	double timePerFrame = 1000.0 / updateRate;
	double timePerRenderFrame = 1000.0 / renderRefreshRate;
	static double timeSinceLastRender = 0.0;
	long lastTime = 0;

	double targetSleep = 0.0;
	double sleepRemainder = 0.0;
	double startSleep = 0.0;
	double sleepTime = 0.0;

	loadConfig();
	initSDL();
	initGlew();
	initBullet();
	initGLContexts();
	initObjects();
	
	running = true;
	HANDLE listenThreadHandle = HostListener::start();

	loadResources();
	loadLevel("asdf");

	while (running) {
		frameCount++;
		if (GetAsyncKeyState(VK_ESCAPE) && (GetForegroundWindow() == consoleWindow) &&
			MessageBox(0, "Shutdown the Server?", "Rune Server Shutdown", MB_YESNO) == IDYES) break;

		lastTime = lastTime = SDL_GetTicks();

		serverEventsTCP->clearSendBuffer();
		serverEventsTCP->clearReceiveBuffer();
		serverEventsUDP->clearSendBuffer();
		serverEventsUDP->clearReceiveBuffer();

		//Get all commands from clients
		for (int i = 0; i < maxPlayers; i++) {
			DataBuffer * temp;
			aquireClientsMutex(i);
				temp = clients[i]->popReceiveData();
				if (!clients[i]->isConnected() && playerObjects->at(i) != NULL) {
					int index = -1;
					int x = 0;
					for (list<pair<unsigned long, GameObject*>>::iterator iter = gameObjects->begin(); iter != gameObjects->end(); iter++) {
						if (playerObjects->at(i) == iter->second) {
							index = x;
							break;
						}
						x++;
					}
					/*for (int x = 0; x < gameObjects->size(); x++) {
						if (playerObjects->at(i) == gameObjects->at(x))
							index = x;
					}*/
					gameObjects->remove(playerObjects->at(i)->getNetID());
					delete playerObjects->at(i);
					playerObjects->at(i) = NULL;
					//gameObjects->erase(gameObjects->begin() + index);
					serverEventsTCP->appendToSendBuffer(NetworkFunctions::createDeleteObjectBuffer(index));
				}
			releaseClientsMutex(i);
			serverEventsTCP->appendToReceiveBuffer(temp);
		}

		//Receive All UDP Events
		sockaddr_in * originAddress = NULL;
		DataBuffer * udpBuffer = UDPManager::recvDatagram(&originAddress);
		while (udpBuffer != NULL) {
			//origin Address matches client address?
			unsigned long packetFrameCount = 0;
			memcpy(&packetFrameCount, udpBuffer->getData(), 4);

			int clientID = getClientIDByAddress(originAddress->sin_addr.s_addr, originAddress->sin_port);
			if (clientID == -1) {
				int asdfadsfasdf = 123123;
			}
			else {
				//compare clientFrameCount to client->frameCount
				unsigned long clientFrameCount;
				aquireClientsMutex(clientID);
					clientFrameCount = clients[clientID]->getFrameCount();
				releaseClientsMutex(clientID);

				if (packetFrameCount < clientFrameCount) {
					//discard
					printf("Packet from Client %d discarded\n", clientID);
				}
				else {
					serverEventsUDP->appendToReceiveBuffer(udpBuffer->getData() + 4, udpBuffer->getSize() - 4);
					aquireClientsMutex(clientID);
						clients[clientID]->setFrameCount(packetFrameCount);
					releaseClientsMutex(clientID);
				}

			}
			
			delete udpBuffer; udpBuffer = NULL;
			delete originAddress; originAddress = NULL;
			udpBuffer = UDPManager::recvDatagram(&originAddress);
		}

		NetworkFunctions::translateReceivedEvents(serverEventsTCP);
		NetworkFunctions::translateReceivedEvents(serverEventsUDP);
		
		//Execute my own gamelogic commands
		SDL_Event e;
		while (SDL_PollEvent(&e)) { 
			if (e.key.keysym.sym == SDLK_ESCAPE)
						running = false;
			if (e.key.keysym.sym == SDLK_m) {
				((RigidObject*)gameObjects->getValue(2))->setPosition(5, 10, 0);
				((RigidObject*)gameObjects->getValue(3))->setPosition(0, 10, 0);
				((RigidObject*)gameObjects->getValue(4))->setPosition(-5, 10, 0);
				((RigidObject*)gameObjects->getValue(2))->setVelocity(0, 0, 0);
				((RigidObject*)gameObjects->getValue(3))->setVelocity(0, 0, 0);
				((RigidObject*)gameObjects->getValue(4))->setVelocity(0, 0, 0);
				((RigidObject*)gameObjects->getValue(2))->setGravity(0, -9.81f*4.0f, 0);
				((RigidObject*)gameObjects->getValue(3))->setGravity(0, -9.81f*4.0f, 0);
				((RigidObject*)gameObjects->getValue(4))->setGravity(0, -9.81f*4.0f, 0);
				printf("reset boxes\n");
			}
		}

		//if (update((long)(elapsedTime + sleepTime)) == -1) break;

		//dynamicsWorld->stepSimulation((float)(elapsedTime + sleepTime)*0.001f, 50, (float)(elapsedTime + sleepTime)*0.001f);
		dynamicsWorld->stepSimulation(1.0f / 64.0f, 100, 1.0f / 64.0f); //1.0f/(float)Settings::getMaxFPS());

		list<pair<unsigned long, GameObject*>>::iterator iter = gameObjects->begin();
		while (iter != gameObjects->end())
		{
			iter->second->update(elapsedTime);
			if (!iter->second->isAlive()) {
				serverEventsTCP->appendToSendBuffer(NetworkFunctions::createDeleteObjectBuffer(iter->second->getNetID()));
				for (int p = 0; p < maxPlayers; p++) {
					if (playerObjects->at(p) != NULL && playerObjects->at(p) == iter->second) {
						playerObjects->at(p) = NULL;
					}
				}

				GameObject * tempObject = iter->second;
				iter = gameObjects->erase(iter);
				delete tempObject;
			}
			else iter++;
		}

		/*for (list<pair<unsigned long, GameObject*>>::iterator iter = gameObjects->begin(); iter != gameObjects->end(); iter++) {
			iter->second->update(elapsedTime);
			if (!iter->second->isAlive()) {
				serverEventsTCP->appendToSendBuffer(NetworkFunctions::createDeleteObjectBuffer(iter->second->getNetID()));
				for (int p = 0; p < maxPlayers; p++) {
					if (playerObjects->at(p) != NULL && playerObjects->at(p) == iter->second) {
						playerObjects->at(p) = NULL;
					}
				}

				gameObjects->remove(iter->first, iter->second);
				delete (*gameObjects)[i];
				gameObjects->erase(gameObjects->begin() + i);
				i--;
			}
		}*/

		/*for (int i = 0; i < (int)gameObjects->size(); i++) {
			(*gameObjects)[i]->update(elapsedTime);
			if (!(*gameObjects)[i]->isAlive()) {
				serverEventsTCP->appendToSendBuffer(NetworkFunctions::createDeleteObjectBuffer(i));
				for (int p = 0; p < maxPlayers; p++) {
					if (playerObjects->at(p) != NULL && playerObjects->at(p) == (*gameObjects)[i]) {
						playerObjects->at(p) = NULL;
					}
				}
				delete (*gameObjects)[i];
				gameObjects->erase(gameObjects->begin() + i);
				i--;
			}
		}*/
		
		//Update each client
		//for (int i = 0; i < gameObjects->size(); i++)
		//	serverEvents->appendToSendBuffer(NetworkFunctions::createUpdateObjectBuffer(i));

		for (int i = 0; i < maxPlayers; i++) {
			aquireClientsMutex(i);
				if (clients[i]->isConnected()) {
					if (clients[i]->requiresSynch()) {
						synchClient(i);
						clients[i]->setSynch(false);
					}
					else {
						clients[i]->pushSendData(serverEventsTCP->getSendBuffer(), serverEventsTCP->getSendBufferSize());
					}
				}
			releaseClientsMutex(i);
		}
		
		for (int i = 0; i < maxPlayers; i++) {
			aquireClientsMutex(i);
			if (clients[i]->isConnected()) {
				//unsigned long clientPortUDP = clients[i]->getPortUDP();
				if (!clients[i]->requiresSynch()) {
					serverEventsUDP->clearSendBuffer();
					for (list<pair<unsigned long, GameObject*>>::iterator iter = gameObjects->begin(); iter != gameObjects->end(); iter++) {
						if (playerObjects->at(i) != iter->second /* && dont force player */) {
							DataBuffer * temp = NetworkFunctions::createUpdateObjectBuffer(iter->second->getNetID());
							serverEventsUDP->appendToSendBuffer(temp);
						}
					}
					UDPManager::sendDatagram(serverEventsUDP->getSendBuffer(), serverEventsUDP->getSendBufferSize(), clients[i]->getIPAddress(), clients[i]->getPortUDP(), frameCount);
				}
			}
			releaseClientsMutex(i);
		}

		/*timeSinceLastRender += elapsedTime;
		if (timeSinceLastRender >= timePerRenderFrame) {
			timeSinceLastRender = 0.0;
			renderFrame();
		}*/
		renderFrame();

		//Update Rate Calculations
		//Framerate Limit Calculations
		elapsedTime = SDL_GetTicks() - lastTime;
		targetSleep = timePerFrame - elapsedTime + sleepRemainder;
		if (targetSleep > 0) sleepRemainder = targetSleep - (Uint32)targetSleep;

		//Sleep(1000.0 / 64.0);
		
		startSleep = SDL_GetTicks();
		while ((Uint32)(startSleep + targetSleep) > SDL_GetTicks());
		sleepTime = SDL_GetTicks() - startSleep;
	}

	cout << "Shutting Down\n";
	HostThread::stopAll();
	HostListener::stop();
	UDPManager::cleanup();
	WSACleanup();
	WaitForSingleObject(listenThreadHandle, INFINITE);

	saveConfig();

	cleanupObjects();
	//saveConfig();
	killBullet();
		
	//delete [] clients;
	
	return 0;
}

int Server::update(long elapsedTime) 
{
	//dynamicsWorld->stepSimulation((float)elapsedTime*0.001f, 50, (float)elapsedTime*0.001f); //1.0f/(float)Settings::getMaxFPS());

	dynamicsWorld->stepSimulation(1.0f / 64.0f, 100, 1.0f / 64.0f); //1.0f/(float)Settings::getMaxFPS());

	list<pair<unsigned long, GameObject*>>::iterator objectsIter = gameObjects->begin();
	while (objectsIter != gameObjects->end())
	{
		objectsIter->second->update(elapsedTime);
		if (!objectsIter->second->isAlive()) {
			GameObject * tempObject = objectsIter->second;
			objectsIter = gameObjects->erase(objectsIter);
			delete tempObject;
			//Send Erase Command for i
		}
		else objectsIter++;
	}

	return 0;
}

int Server::renderFrame()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	SDL_GL_SwapWindow(displayWindow);
	return 0;
}

void Server::loadConfig()
{
	if (!dirExists("Config")) {
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, false };
		CreateDirectory("Config", &sa);
	}

	FILE * file;
	char configName[] = "Config\\Server Settings.ini";
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

		if (var == "tickRate")				updateRate = atoi(val.c_str());
		else if (var == "windowWidth")		Settings::setWindowWidth(atoi(val.c_str()));
		else if (var == "windowHeight")		Settings::setWindowHeight(atoi(val.c_str()));
		else if (var == "serverPortTCP")	Settings::setPortTCP(atoi(val.c_str()));
		else if (var == "serverPortUDP")	Settings::setPortUDP(atoi(val.c_str()));
	}

	fileStream.close();
}

void Server::saveConfig()
{
	ofstream output;
	char configName[] = "Config\\Server Settings.ini";
	output.open(configName);

	const unsigned char * ver = glGetString(GL_VERSION);
	string v = "";
	int i = 0;
	while (ver[i] != ' ') { v += ver[i]; i++; }

	float version = (float)atof(v.c_str());

	output
		<< "tickRate=" << updateRate << '\n'
		<< "windowWidth=" << Settings::getWindowWidth() << '\n'
		<< "windowHeight=" << Settings::getWindowHeight() << '\n'
		<< "serverPortTCP="	<< Settings::getPortTCP() << '\n'
		<< "serverPortUDP="	<< Settings::getPortUDP();

	output.close();
}

int Server::loadResources()
{
	//assetManager->forceLoadModel("Models\\testramp.obj");

	return 0;
}

int Server::loadLevel(string asdf)
{
	clearObjects();
	//initTestCube();

	assetManager->forceLoadModel("Models\\testramp.obj");
	new MeshObject("testramp.obj", "rock_redDeepCuts_2k_d.tga", "default.glsl",
		glm::vec3(0, -10, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));

	PropObject * temp = new PropObject("box.obj", "error.tga", "default.glsl", glm::vec3(0, 10,0), glm::vec3(0,0,1), glm::vec3(0,1,0), "box1x1.prop");
	temp->setGravity(glm::vec3(0, 0, 0));
	temp = new PropObject("box.obj", "error.tga", "default.glsl", glm::vec3(5, 10, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), "box1x1.prop");
	temp->setGravity(glm::vec3(0, 0, 0));
	temp = new PropObject("box.obj", "error.tga", "default.glsl", glm::vec3(-5, 10, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), "box1x1.prop");
	temp->setGravity(glm::vec3(0, 0, 0));

	new FullHealthKit(glm::vec3(70, -10, 0));

	/*CharacterObject * temp = new CharacterObject("box.obj", "error.tga", "default.glsl",
		glm::vec3(1, 2, 3), glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), "box1x1.prop", 0);
	playerObjects->at(0) = temp;

	DataBuffer * tempBuffer = NetworkFunctions::createCreateObjectBuffer(1);
	NetworkFunctions::executeIndexedFunction(tempBuffer);
	delete tempBuffer;*/
	
	return 0;
}

int Server::initGlew()
{
	static bool alreadyExecuted = false;
	if (alreadyExecuted) return -1;
	alreadyExecuted = true;

	const bool loggingExtensions = true; //Set to true to have extensions logged...
	char *extensions = (char *)glGetString(GL_EXTENSIONS);

	//bool isARBVertexBufferObjectExtensionPresent = isExtensionSupported (extensions, "GL_ARB_vertex_buffer_object");
	//if (isARBVertexBufferObjectExtensionPresent) {
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
	glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
	glIsBufferARB = (PFNGLISBUFFERARBPROC)wglGetProcAddress("glIsBufferARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
	glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)wglGetProcAddress("glBufferSubDataARB");
	glGetBufferSubDataARB = (PFNGLGETBUFFERSUBDATAARBPROC)wglGetProcAddress("glGetBufferSubDataARB");
	glMapBufferARB = (PFNGLMAPBUFFERARBPROC)wglGetProcAddress("glMapBufferARB");
	glUnmapBufferARB = (PFNGLUNMAPBUFFERARBPROC)wglGetProcAddress("glUnmapBufferARB");
	glGetBufferParameterivARB = (PFNGLGETBUFFERPARAMETERIVARBPROC)wglGetProcAddress("glGetBufferParameterivARB");
	glGetBufferPointervARB = (PFNGLGETBUFFERPOINTERVARBPROC)wglGetProcAddress("glGetBufferPointervARB");
	//}

	//bool isGL_ARB_shader_objectsExtensionPresent = isExtensionSupported (extensions, "GL_ARB_shader_objects");
	//if (isGL_ARB_shader_objectsExtensionPresent) {
	glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)wglGetProcAddress("glAttachObjectARB");
	glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShaderARB");
	glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgramObjectARB");
	glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShaderObjectARB");
	glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)wglGetProcAddress("glDeleteObjectARB");
	glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)wglGetProcAddress("glDetachObjectARB");
	glGetActiveUniformARB = (PFNGLGETACTIVEUNIFORMARBPROC)wglGetProcAddress("glGetActiveUniformARB");
	glGetAttachedObjectsARB = (PFNGLGETATTACHEDOBJECTSARBPROC)wglGetProcAddress("glGetAttachedObjectsARB");
	glGetHandleARB = (PFNGLGETHANDLEARBPROC)wglGetProcAddress("glGetHandleARB");
	glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
	glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC)wglGetProcAddress("glGetObjectParameterfvARB");
	glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
	glGetShaderSourceARB = (PFNGLGETSHADERSOURCEARBPROC)wglGetProcAddress("glGetShaderSourceARB");
	glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocationARB");
	glGetUniformfvARB = (PFNGLGETUNIFORMFVARBPROC)wglGetProcAddress("glGetUniformfvARB");
	glGetUniformivARB = (PFNGLGETUNIFORMIVARBPROC)wglGetProcAddress("glGetUniformivARB");
	glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)wglGetProcAddress("glLinkProgramARB");
	glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)wglGetProcAddress("glShaderSourceARB");
	glUniform1fARB = (PFNGLUNIFORM1FARBPROC)wglGetProcAddress("glUniform1fARB");
	glUniform1fvARB = (PFNGLUNIFORM1FVARBPROC)wglGetProcAddress("glUniform1fvARB");
	glUniform1iARB = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1iARB");
	glUniform1ivARB = (PFNGLUNIFORM1IVARBPROC)wglGetProcAddress("glUniform1ivARB");
	glUniform2fARB = (PFNGLUNIFORM2FARBPROC)wglGetProcAddress("glUniform2fARB");
	glUniform2fvARB = (PFNGLUNIFORM2FVARBPROC)wglGetProcAddress("glUniform2fvARB");
	glUniform2iARB = (PFNGLUNIFORM2IARBPROC)wglGetProcAddress("glUniform2iARB");
	glUniform2ivARB = (PFNGLUNIFORM2IVARBPROC)wglGetProcAddress("glUniform2ivARB");
	glUniform3fARB = (PFNGLUNIFORM3FARBPROC)wglGetProcAddress("glUniform3fARB");
	glUniform3fvARB = (PFNGLUNIFORM3FVARBPROC)wglGetProcAddress("glUniform3fvARB");
	glUniform3iARB = (PFNGLUNIFORM3IARBPROC)wglGetProcAddress("glUniform3iARB");
	glUniform3ivARB = (PFNGLUNIFORM3IVARBPROC)wglGetProcAddress("glUniform3ivARB");
	glUniform4fARB = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4fARB");
	glUniform4fvARB = (PFNGLUNIFORM4FVARBPROC)wglGetProcAddress("glUniform4fvARB");
	glUniform4iARB = (PFNGLUNIFORM4IARBPROC)wglGetProcAddress("glUniform4iARB");
	glUniform4ivARB = (PFNGLUNIFORM4IVARBPROC)wglGetProcAddress("glUniform4ivARB");
	glUniformMatrix2fvARB = (PFNGLUNIFORMMATRIX2FVARBPROC)wglGetProcAddress("glUniformMatrix2fvARB");
	glUniformMatrix3fvARB = (PFNGLUNIFORMMATRIX3FVARBPROC)wglGetProcAddress("glUniformMatrix3fvARB");
	glUniformMatrix4fvARB = (PFNGLUNIFORMMATRIX4FVARBPROC)wglGetProcAddress("glUniformMatrix4fvARB");
	glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)wglGetProcAddress("glUseProgramObjectARB");
	glValidateProgramARB = (PFNGLVALIDATEPROGRAMARBPROC)wglGetProcAddress("glValidateProgramARB");

	// Other Shader Stuff
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
	glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)wglGetProcAddress("glGetAttachedShaders");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
	glUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
	glUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	//}
	glewInit();

	return 0;
}

int Server::initOpenGL()
{
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping ( NEW )
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);  // clockwise oriented polys are front faces
	glCullFace(GL_BACK); // cull out the inner polygons... Set Culling property appropriately

	glShadeModel(GL_SMOOTH);							// Enable smooth shading
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Set the background black
	glClearDepth(1.0f);								// Depth buffer setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);							// The Type Of Depth Test To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);			// filled polys
	
	return 0;
}

int Server::resizeWindow(int width, int height) {
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	if (width == desktop.right && height == desktop.bottom)
		width++;

	SDL_SetWindowSize(displayWindow, width, height);

	//Setup a new viewport.
	glViewport(0, 0, width, height);
	Settings::setWindowWidth(width);
	Settings::setWindowHeight(height);

	//Setup a new perspective matrix.
	GLdouble verticalFieldOfViewInDegrees = 40;
	GLdouble aspectRatio = height == 0 ? 1.0 : (GLdouble)width / (GLdouble)height;
	GLdouble nearDistance = 1.0;
	GLdouble farDistance = 2000.0;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(verticalFieldOfViewInDegrees, aspectRatio, nearDistance, farDistance);

	//Get back to default mode.
	glMatrixMode(GL_MODELVIEW);

	return 0;
}

int Server::initSDL()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	Uint32 windowFlags;

	//if (Settings::getWindowState() == FULLSCREEN)		windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN;
	windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
	//else if (Settings::getWindowState() == BORDERLESS) windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_BORDERLESS;

	displayWindow = SDL_CreateWindow("Rune Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, windowFlags);

	displayContext = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, displayContext);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);

	initOpenGL();
	//resizeWindow(800, 600);

	SDL_SetRelativeMouseMode(SDL_FALSE);

	return 0;
}

int Server::initGLContexts()
{
	diplayWindow = wglGetCurrentDC();
	primaryContext = wglGetCurrentContext();
	loadingThreadContext = wglCreateContext(diplayWindow);
	wglMakeCurrent(NULL, NULL);
	BOOL error = wglShareLists(primaryContext, loadingThreadContext);
	wglMakeCurrent(diplayWindow, primaryContext);

	return 0;
}

int Server::initObjects()
{
	gameObjects = new LinkedHashMap<unsigned long, GameObject*>();
	playerObjects = new vector<CharacterObject*>();
	playerObjects->reserve(maxPlayers);
	for (int i = 0; i < maxPlayers; i++) {
		playerObjects->push_back(NULL);
	}
	
	serverEventsTCP = new Packet();
	serverEventsUDP = new Packet();

	assetManager = new AssetManager();
	assetManager->setStatics(&diplayWindow, &loadingThreadContext, &primaryContext);
	RenderObject::setStatics(&view, &projection, assetManager, serverEventsTCP, serverEventsUDP, dynamicsWorld);
	RenderObject::setNetworkState(NETWORK_STATE_SERVER);
	GameObject::setStatics(gameObjects);
	RigidObject::populateCollisionLibrary();
	PhysicsObject::setStatics(&collisionShapes);
	HostThread::setStatics(&maxPlayers, &clients);
	NetworkFunctions::setNetworkState(NETWORK_STATE_SERVER);
	NetworkFunctions::setStatics(gameObjects, playerObjects);
	NetworkFunctions::setEventPackets(serverEventsTCP, serverEventsUDP);
	UDPManager::init(Settings::getPortUDP());

	return 0;
}

int Server::clearObjects()
{
	while (gameObjects->size() > 0) {
		delete gameObjects->backValue();
		gameObjects->popBack();
	}

	for (int i = 0; i < playerObjects->size(); i++)
		playerObjects->at(i) = NULL;

	cleanBullet();

	GameObject::resetNextNetID();

	return 0;
}

int Server::cleanupObjects()
{
	for (unsigned int i = 0; i < clients.size(); i++)
		delete clients[i];
	
	clearObjects();

	delete serverEventsTCP;
	delete serverEventsUDP;

	delete playerObjects;
	delete gameObjects;
	//deadObjects->clear();
	//delete deadObjects;

	RigidObject::deleteCollisionLibrary();
	assetManager->shutdown();
	delete assetManager;
	return 0;
}

bool Server::myContactProcessedCallback(btManifoldPoint& cp, void * body0, void * body1)
{
	if (((btRigidBody*)body0)->getOwner() != NULL)
		((RigidObject*)((btRigidBody*)body0)->getOwner())->collisionCallback((RigidObject*)((btRigidBody*)body1)->getOwner());
	if (((btRigidBody*)body1)->getOwner() != NULL)
		((RigidObject*)((btRigidBody*)body1)->getOwner())->collisionCallback((RigidObject*)((btRigidBody*)body0)->getOwner());
	return true;
}

int Server::initBullet()
{
	/// collision configuration contains default setup for memory , collision setup . Advanced users can create their own configuration .
	collisionConfiguration = new btDefaultCollisionConfiguration();

	///use the default collision dispatcher . For parallel processing you can use a diffent dispatcher ( see Extras / BulletMultiThreaded )
	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	/// btDbvtBroadphase is a good general purpose broadphase . You can also try out btAxis3Sweep .
	broadphase = new btDbvtBroadphase();
	//broadphase = new btAxisSweep3 ();

	///the default constraint solver . For parallel processing you can use a different solver ( see Extras / BulletMultiThreaded )
	solver = new btSequentialImpulseConstraintSolver;

	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
	//ghostPairCallback = new btGhostPairCallback();
	//dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(ghostPairCallback);
	dynamicsWorld->setGravity(btVector3(0, -9.81f*4.0f, 0));
	gContactProcessedCallback = (ContactProcessedCallback)myContactProcessedCallback;

	return 0;
}

int Server::cleanBullet()
{
	dynamicsWorld->clearForces();

	for (int i = dynamicsWorld->getNumConstraints() - 1; i >= 0; i--) {
		btTypedConstraint * cons = dynamicsWorld->getConstraint(i);
		dynamicsWorld->removeConstraint(cons);
	}

	for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
		btCollisionObject * obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody * body = btRigidBody::upcast(obj);
		if (body && body->getMotionState()) {
			delete body->getMotionState();
		}
		dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	for (int j = 0; j < collisionShapes.size(); j++) {
		btCollisionShape * shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	return 0;
}

int Server::killBullet()
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

void Server::readServerData(string iniPath)
{
	//read values from iniPath
	// .
	// .
	// .
}

int Server::aquireClientsMutex(int index)
{
	if (index > maxPlayers) {
		printf("Client Mutex Index Out of bounds.\n");
		return -1;
	}
	return clients[index]->aquireMutex();
}

int Server::releaseClientsMutex(int index)
{
	if (index > maxPlayers) {
		printf("Client Mutex Index Out of bounds.\n");
		return -1;
	}
	return clients[index]->releaseMutex();
}

int Server::synchClient(int clientID)
{
	//aquireClientsMutex(clientID);
		//Clear Objects
		DataBuffer * temp = NetworkFunctions::createClearObjectsBuffer();
		clients[clientID]->pushSendData(temp->getData(), temp->getSize());
		delete temp;

		//Add Existing Objects
		for (list<pair<unsigned long, GameObject*>>::iterator iter = gameObjects->begin(); iter != gameObjects->end(); iter++) {
			temp = NetworkFunctions::createCreateObjectBuffer(iter->second);
			clients[clientID]->pushSendData(temp->getData(), temp->getSize());
			delete temp;
		}
		/*for (int i = 0; i < gameObjects->size(); i++) {
			temp = NetworkFunctions::createCreateObjectBuffer(gameObjects->at(i));
			clients[clientID]->pushSendData(temp->getData(), temp->getSize());
			delete temp;
		}*/

		for (int i = 0; i < playerObjects->size(); i++) {
			if (playerObjects->at(i) != NULL) {
				//Add All Existing Spells to players
				/*for (int elementIndex = 0; elementIndex < 5; elementIndex++) {
					for (int subIndex = 0; subIndex < playerObjects->at(i)->getSpells()[elementIndex].size(); subIndex++) {
						Spell * tempSpell = playerObjects->at(i)->getSpell(elementIndex, subIndex);
						if (tempSpell != NULL) {
							temp = NetworkFunctions::createAddSpellBuffer(clientID, elementIndex, tempSpell->getSpellID());
							clients[clientID]->pushSendData(temp->getData(), temp->getSize());
						}
					}
				}*/

				//Change Spell to active one
				int currentElementIndex, currentSubIndex;
				playerObjects->at(i)->getActiveSpellIndices(&currentElementIndex, &currentSubIndex);
				temp = NetworkFunctions::createChangeSpellBuffer(i, currentElementIndex, currentSubIndex);
				clients[clientID]->pushSendData(temp->getData(), temp->getSize());
			}
		}
		//send all the items to create
	//releaseClientsMutex(clientID);

	return 0;
}

int Server::getClientIDByAddress(unsigned long ipAddress, unsigned long port)
{
	for (int i = 0; i < clients.size(); i++) {
		clients[i]->aquireMutex();
			if (clients[i]->getIPAddress() == ipAddress && clients[i]->getPortUDP() == port) {
				clients[i]->releaseMutex();
				return i;
			}
		clients[i]->releaseMutex();
	}
	return -1;
}