#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

class Packet;

#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <stdio.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"	

#include "EventDeclarations.h"
#include "AssetManager.h"
#include "Utilities.h"
#include "Camera.h"
#include "Packet.h"
using namespace Utilities;

class RenderObject
{
public:
	RenderObject();
	RenderObject(string, string, string); //ok for now
	RenderObject(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up); //ok for now
	virtual ~RenderObject();

	virtual int update(long elapsedTime);
	virtual int render(long totalElapsedTime);
	virtual int shaderlessRender(long totalElapsedTime, bool useOwnTexture = false);
	virtual int updateMatrices();

	virtual void setDirection(glm::vec3 x) { direction = x; updateMatrices(); }
	virtual void setDirection(float x, float y, float z) { setDirection(glm::vec3(x, y, z)); }
	virtual void setScale(glm::vec3 x) { scale = x; updateMatrices(); }
	virtual void setScale(float x, float y, float z) { setScale(glm::vec3(x, y, z)); }
	virtual void setPosition(glm::vec3 x) { position = x; updateMatrices(); }
	virtual void setPosition(float x, float y, float z) { setPosition(glm::vec3(x, y, z)); }

	virtual glm::vec3 getPosition() { return position; }
	virtual glm::vec3 getDirection() { return direction; }
	virtual glm::vec3 getUp() { return up; }

	int setShaderParameter(string paramName, glm::vec4 val);

	static void setStatics(glm::mat4 * view, glm::mat4 * proj, AssetManager * ass, Packet * tcp, Packet * udp, btDiscreteDynamicsWorld * dyn)
	{ viewMatrix = view; projMatrix = proj; assetManager = ass; eventsTCP = tcp; eventsUDP = udp; dynamicsWorld = dyn; }
	static void setNetworkState(int x) { networkState = x; }

	virtual string getTypeName() { return "RenderObject"; }
	string getModelPath() { return "Models\\" + modelName; }
	string getModelName() { return modelName.substr(0, modelName.length() - 4); } //.obj
	string getTexturePath() { return "Textures\\" + textureName; }
	string getTextureName() { return textureName.substr(0, textureName.length() - 4); }	//.tga
	string getShaderPath() { return "Shaders\\" + shaderName; }
	string getShaderName() { return shaderName.substr(0, shaderName.length() - 5); } //.glsl

protected:
	string modelName;
	string textureName;
	string shaderName;

	Model * getModel(string path) { return assetManager->getModel(path); }
	Texture * getTexture(string path) { return assetManager->getTexture(path); }
	Shader * getShader(string path) { return assetManager->getShader(path); }
	
	Model * model;
	Texture * texture;
	Shader * shader;

	bool lighting;

	glm::mat4 worldMatrix, normalMatrix;
	glm::vec3 position, direction, up, scale;
	
	#define activeCamera Camera::getActiveCamera()
	#define activeShader Shader::getActiveShader()

	int addTCPEvent(DataBuffer * data);
	int addUDPEvent(DataBuffer * data);

	static glm::mat4 * viewMatrix;
	static glm::mat4 * projMatrix;

	static btDiscreteDynamicsWorld * dynamicsWorld;
	static int getNetworkState() { return networkState; }
private:
	void init();

	static AssetManager * assetManager;
	static Packet * eventsTCP;
	static Packet * eventsUDP;
	static int networkState;
	//static bool connectedToHost;
};

#endif