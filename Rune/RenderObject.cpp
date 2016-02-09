#include "RenderObject.h"

glm::mat4 * RenderObject::viewMatrix = NULL;
glm::mat4 * RenderObject::projMatrix = NULL;
AssetManager * RenderObject::assetManager = NULL;
Packet * RenderObject::eventsTCP = NULL;
Packet * RenderObject::eventsUDP = NULL;
int RenderObject::networkState = NETWORK_STATE_OFFLINE;

RenderObject::RenderObject()
{
	init();
}

RenderObject::RenderObject(string mName, string tName, string sName)
{
	init();

	modelName = mName;
	textureName = tName;
	shaderName = sName;
	model = assetManager->getModel(getModelPath());
	texture = assetManager->getTexture(getTexturePath());
	shader = assetManager->getShader(getShaderPath());

	//UNTIL DYNAMIC LOADING IS FIXED
	//----------------------------------------------------
	/*if (model == NULL) {
		assetManager->forceLoadModel(modelPath);
		model = assetManager->getModel(modelPath);
	}
	if (texture == NULL) {
		assetManager->forceLoadModel(texturePath);
		texture = assetManager->getTexture(texturePath);
	}
	if (shader == NULL) {
		assetManager->forceLoadShader(shaderPath);
		shader = assetManager->getShader(shaderPath);
	}*/
	//----------------------------------------------------

	position = glm::vec3(0,0,0);
	direction = glm::vec3(0,0,-1);
	up = glm::vec3(0,1,0);
}

RenderObject::RenderObject(string mName, string tName, string sName, glm::vec3 position, glm::vec3 direction, glm::vec3 up)
{
	init();

	modelName = mName;
	textureName = tName;
	shaderName = sName;
	model = assetManager->getModel(getModelPath());
	texture = assetManager->getTexture(getTexturePath());
	shader = assetManager->getShader(getShaderPath());

	//UNTIL DYNAMIC LOADING IS FIXED
	//----------------------------------------------------
	/*if (model == NULL) {
		assetManager->forceLoadModel(modelPath);
		model = assetManager->getModel(modelPath);
	}
	if (texture == NULL) {
		assetManager->forceLoadModel(texturePath);
		texture = assetManager->getTexture(texturePath);
	}
	if (shader == NULL) {
		assetManager->forceLoadShader(shaderPath);
		shader = assetManager->getShader(shaderPath);
	}
	//----------------------------------------------------
	*/
	this->position = position;
	this->direction = direction;
	this->up = up;

	updateMatrices();
}

RenderObject::~RenderObject()
{
	
}

void RenderObject::init()
{
	model = NULL;
	texture = NULL;
	shader = NULL;
	lighting = true;
	position = glm::vec3(0,0,0);
	direction = glm::vec3(0,0,-1);
	up = glm::vec3(0,1,0);
	scale = glm::vec3(1, 1, 1);
}

int RenderObject::update(long elapsedTime)
{
	if (getNetworkState() != NETWORK_STATE_SERVER) {
		if (model == NULL)
			model = assetManager->getModel(getModelPath());
		if (texture == NULL)
			texture = assetManager->getTexture(getTexturePath());
		if (shader == NULL)
			shader = assetManager->getShader(getShaderPath());
	}
	return 0;
}

int RenderObject::updateMatrices() 
{
	worldMatrix = glm::inverse(glm::lookAt(position, position + direction, up)) * glm::scale(scale);
	normalMatrix = glm::transpose(glm::inverse(worldMatrix));
	return 0;
}

int RenderObject::setShaderParameter(string name, glm::vec4 val)
{
	if (shader == NULL) return -1;

	Shader * temp = activeShader;
	shader->activate();

	activeShader->setUniformf4(name, val.x, val.y, val.z, val.w);

	if (temp != NULL) temp->activate(); //restore old shader

	return 0;
}

int RenderObject::render(long totalElapsedTime)
{
	if (shader == NULL || texture == NULL || model == NULL || projMatrix == NULL || viewMatrix == NULL) return -1;;

	Shader * temp = activeShader;

	shader->activate();

	activeShader->setUniformTexture("textureSampler", texture->getTextureID());
	glActiveTexture(GL_TEXTURE0);
	texture->bindTexture();
	glUniform1i(glGetUniformLocation(activeShader->getShaderHandle(), "textureSampler"), 0);

	activeShader->setUniformMatrixf4("projViewMatrix", *projMatrix * *viewMatrix);
	activeShader->setUniformMatrixf4("worldMatrix", worldMatrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);

	if (lighting) {
		glm::vec3 camPos = activeCamera->getPos();
		shader->setUniformf3("cameraPosition", camPos.x, camPos.y, camPos.z);
		activeShader->setUniformf1("enableLighting", 1.0f);
	}
	else activeShader->setUniformf1("enableLighting", 0.0f);
		
	model->render();

	if (temp != NULL) temp->activate(); //restore old shader

	return 0;
}

int RenderObject::shaderlessRender(long totalElapsedTime, bool useOwnTexture)
{
	if (texture == NULL || model == NULL || projMatrix == NULL || viewMatrix == NULL) return -1;;

	if (useOwnTexture) {
		activeShader->setUniformTexture("textureSampler", texture->getTextureID());
		glActiveTexture(GL_TEXTURE0);
		texture->bindTexture();
		glUniform1i(glGetUniformLocation(activeShader->getShaderHandle(), "textureSampler"), 0);
	}

	activeShader->setUniformMatrixf4("projViewMatrix", *projMatrix * *viewMatrix);
	activeShader->setUniformMatrixf4("worldMatrix", worldMatrix);
	activeShader->setUniformMatrixf4("normalMatrix", normalMatrix);

	if (lighting) {
		glm::vec3 camPos = activeCamera->getPos();
		activeShader->setUniformf3("cameraPosition", camPos.x, camPos.y, camPos.z);
		activeShader->setUniformf1("enableLighting", 1.0f);
	}
	else activeShader->setUniformf1("enableLighting", 0.0f);
	
	model->render();

	return 0;
}

int RenderObject::addTCPEvent(DataBuffer * data)
{
	if (eventsTCP == NULL) return -1;
	eventsTCP->appendToSendBuffer(data);
	return 0;
}

int RenderObject::addUDPEvent(DataBuffer * data)
{
	if (eventsUDP == NULL) return -1;
	eventsUDP->appendToSendBuffer(data);
	return 0;
}