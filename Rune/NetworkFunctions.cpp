#include "NetworkFunctions.h"

int NetworkFunctions::clientID = -1;
LinkedHashMap<unsigned long, GameObject*> * NetworkFunctions::gameObjects = NULL;
vector<CharacterObject*> * NetworkFunctions::players = NULL;
Packet * NetworkFunctions::eventPacketTCP = NULL;
Packet * NetworkFunctions::eventPacketUDP = NULL;
int NetworkFunctions::networkState = NETWORK_STATE_SERVER;

bool NetworkFunctions::connectedToHost = false;
FunctionPointer NetworkFunctions::functionArray[] =
{
	clearObjects,				//0
	addPropObject,				//1
	addMeshObject,				//2
	spawnPlayer,				//3
	killGameObject,				//4
	deleteGameObject,			//5	
	updateGameObject,			//6
	addSpell,					//7
	changeSpell,				//8
	attack,						//9
	NULL,						//10 \n char
	damage,						//11
	heal,						//12
	force,						//13
	centralForce,				//14
	spawnFullHealthKit,			//15
	spawnHalfHealthKit,			//16
	createFireball,				//17
	createGrenade,				//18
	createFadingBeamEffect,		//19
};
string NetworkFunctions::functionNames[] =
{
	"clearObjects",				//0
	"addPropObject",			//1
	"addMeshObject",			//2
	"spawnPlayer",				//3
	"killGameObject",			//4
	"deleteGameObject",			//5	
	"updateGameObject",			//6
	"addSpell",					//7
	"changeSpell",				//8
	"attack",					//9
	"NULL",						//10 \n char
	"damage",					//11
	"heal",						//12
	"force",					//13
	"centralForce",				//14
	"spawnFullHealthKit",		//15
	"spawnHalfHealthKit",		//16
	"createFireball",			//17
	"createGrenade",			//18
	"createFadingBeamEffect",	//19
};
int NetworkFunctions::bufferSizes[] = 
{
	5,		//0
	161,	//1
	129,	//2
	165,	//3
	9,		//4
	9,		//5
	-1,		//6, Variable (check game object type)
	17,		//7
	17,		//8
	37,		//9
	0,		//10 \n char
	21,		//11
	21,		//12
	41,		//13
	29,		//14
	21,		//15
	21,		//16
	33,		//17
	33,		//18
	45,		//19
};

int NetworkFunctions::translateReceivedEvents(Packet * serverEvents)
{
	while (serverEvents->getReceiveBufferSize() > 0) {
		DataBuffer * tempBuffer = new DataBuffer(MAX_PACKET_SIZE);

		int charCount = 0;
		int functionIndex = -1;
		int bufferLength = -1;
		for (int i = 0; i < serverEvents->getReceiveBufferSize(); i++) {
			tempBuffer->copy(tempBuffer->getSize(), serverEvents->getReceiveBuffer() + i, 1);

			if (charCount == 3) {
				memcpy(&functionIndex, tempBuffer->getData(), 4);
				if (functionIndex < 0 || functionIndex >= FUNCTION_COUNT || functionIndex == 10) {
					while ((serverEvents->getReceiveBuffer()[i]) != '\n') i++;
					tempBuffer->clear();
					continue;
				}
				else {
					bufferLength = bufferSizes[functionIndex];
				}
			}

			if (charCount == 7 && functionIndex == EVENT_UPDATE_GAME_OBJECT) {
				unsigned long netID = 0;
				memcpy(&netID, tempBuffer->getData() + 4, 4);
				GameObject * tempObject = gameObjects->getValue(netID);
				if (tempObject == NULL) {
					while ((serverEvents->getReceiveBuffer()[i]) != '\n') i++;
					tempBuffer->clear();
					continue;
				}
				/*int objectIndex = -1;
				memcpy(&objectIndex, tempBuffer->getData() + 4, 4);
				if (objectIndex < 0 || objectIndex >= gameObjects->size()) {
					while ((serverEvents->getReceiveBuffer()[i]) != '\n') i++;
					tempBuffer->clear();
					continue;
				}*/
				else {
					bufferLength = tempObject->getSerializedSize();
				}
			}

			if (charCount == bufferLength-1) {
				if (tempBuffer->getSize() == 0) {
					tempBuffer->clear();
					continue;
				}

				executeIndexedFunction(tempBuffer);
				tempBuffer->clear();
				charCount = 0;
				continue;
			}

			/*if (serverEvents->getReceiveBuffer()[i] == '\n') {
				if (tempBuffer->getSize() == 0) {
					tempBuffer->clear();
					continue;
				}

				executeIndexedFunction(tempBuffer);

				tempBuffer->clear();
			}*/
			charCount++;
		}
		delete tempBuffer;
		serverEvents->refillReceiveBuffer();
	}
	return 0;
}

int NetworkFunctions::executeIndexedFunction(DataBuffer * data)
{
	int index;
	memcpy(&index, data->getData(), 4);

	if (index < 0 || index >= FUNCTION_COUNT) return -1;
	//printf("Attempting to Execute Function: %s", functionNames[index]);
	
	return functionArray[index](data);
}

int NetworkFunctions::clearObjects(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 5) return -1;

	while (gameObjects->size() > 0) {
		delete gameObjects->backValue();
		gameObjects->popBack();
	}
	/*for (int i = 0; i < gameObjects->size(); i++)
		delete (*gameObjects)[i];
	gameObjects->clear();*/
	for (int i = 0; i < players->size(); i++)
		(*players)[i] = NULL;

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer
	return 0;
}

DataBuffer * NetworkFunctions::createClearObjectsBuffer()
{
	DataBuffer * tempBuffer = new DataBuffer(5);
	int functionID = EVENT_CLEAR_OBJECTS;
	tempBuffer->copy(0, &functionID, 4);
	tempBuffer->copy(4, "\n", 1);
	return tempBuffer;
}

//Prop Object, 161
/*functionID	 4
netID			 4
modelName		32
textureName		32
shaderName		32
posX			 4
posY			 4
posZ			 4
lookAtX			 4
lookAtY			 4
lookAtZ			 4
collName		32
\n				 1
*/

//Mesh Object, 129
/*functionID	 4
netID			 4
modelName		32
textureName		32
shaderName		32
posX			 4
posY			 4
posZ			 4
lookAtX			 4
lookAtY			 4
lookAtZ			 4
\n				 1
*/

//Character Object, 165
/*functionID	 4
netID			 4
clientID		 4
modelName		32
textureName		32
shaderName		32
posX			 4
posY			 4
posZ			 4
lookAtX			 4
lookAtY			 4
lookAtZ			 4
collName		32
\n				 1
*/

int NetworkFunctions::addPropObject(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 161) return -1;

	unsigned long netID;
	memcpy(&netID, &data->getData()[4], 4);

	char modelName[32];
	//for (int i = 0; i < 32; i++) modelName[i] = '\0';
	memset(modelName, '\0', 32);
	memcpy(modelName, &data->getData()[8], 32);

	char textureName[32];
	//for (int i = 0; i < 32; i++) textureName[i] = '\0';
	memset(textureName, '\0', 32);
	memcpy(textureName, &data->getData()[40], 32);

	char shaderName[32];
	//for (int i = 0; i < 32; i++) shaderName[i] = '\0';
	memset(shaderName, '\0', 32);
	memcpy(shaderName, &data->getData()[72], 32);

	float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
	memcpy(&posX, &data->getData()[104], 4);
	memcpy(&posY, &data->getData()[108], 4);
	memcpy(&posZ, &data->getData()[112], 4);

	float lookAtX = 0.0f, lookAtY = 0.0f, lookAtZ = 0.0f;
	memcpy(&lookAtX, &data->getData()[116], 4);
	memcpy(&lookAtY, &data->getData()[120], 4);
	memcpy(&lookAtZ, &data->getData()[124], 4);

	char propName[32];
	//for (int i = 0; i < 32; i++) propName[i] = '\0';
	memset(propName, '\0', 32);
	memcpy(propName, &data->getData()[128], 32);

	//160 is '\n'
	
	new PropObject(string(modelName) + ".obj", string(textureName) + ".tga", string(shaderName) + ".glsl", 
		glm::vec3(posX, posY, posZ), glm::vec3(lookAtX, lookAtY, lookAtZ), glm::vec3(0, 1, 0), string(propName) + ".prop", netID);

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer
	return 0;
}

DataBuffer * NetworkFunctions::createCreatePropObjectBuffer(PropObject * propObject)
{
	if (propObject == NULL) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(propObject->getCreationSize()); //should be 161

	int functionID = EVENT_ADD_PROP_OBJECT;
	tempBuffer->copy(0, &functionID, 4);
	unsigned long netID = propObject->getNetID();
	tempBuffer->copy(4, &netID, 4);

	string modelName = propObject->getModelName();
	string textureName = propObject->getTextureName();
	string shaderName = propObject->getShaderName();
	string propName = propObject->getCollisionName();

	char nullBuffer[32]; memset(nullBuffer, '\0', 32); //for (int i = 0; i < 32; i++) nullBuffer[i] = '\0';

	tempBuffer->copy(8, nullBuffer, 32);
	tempBuffer->copy(8, modelName.c_str(), std::min(32, (int)modelName.length()));
	tempBuffer->copy(40, nullBuffer, 32);
	tempBuffer->copy(40, textureName.c_str(), std::min(32, (int)textureName.length()));
	tempBuffer->copy(72, nullBuffer, 32);
	tempBuffer->copy(72, shaderName.c_str(), std::min(32, (int)shaderName.length()));

	glm::vec3 pos = propObject->getPosition();
	tempBuffer->copy(104, &pos.x, 4);
	tempBuffer->copy(108, &pos.y, 4);
	tempBuffer->copy(112, &pos.z, 4);

	glm::vec3 lookAt = propObject->getDirection();
	tempBuffer->copy(116, &lookAt.x, 4);
	tempBuffer->copy(120, &lookAt.y, 4);
	tempBuffer->copy(124, &lookAt.z, 4);

	tempBuffer->copy(128, nullBuffer, 32);
	tempBuffer->copy(128, propName.c_str(), std::min(32, (int)propName.length()));
	
	tempBuffer->copy(160, "\n", 1);

	return tempBuffer;
}

int NetworkFunctions::addMeshObject(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 129) return -1;

	unsigned long netID;
	memcpy(&netID, &data->getData()[4], 4);

	char modelName[32];
	//for (int i = 0; i < 32; i++) modelName[i] = '\0';
	memset(modelName, '\0', 32);
	memcpy(modelName, &data->getData()[8], 32);

	char textureName[32];
	//for (int i = 0; i < 32; i++) textureName[i] = '\0';
	memset(textureName, '\0', 32);
	memcpy(textureName, &data->getData()[40], 32);

	char shaderName[32];
	//for (int i = 0; i < 32; i++) shaderName[i] = '\0';
	memset(shaderName, '\0', 32);
	memcpy(shaderName, &data->getData()[72], 32);

	float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
	memcpy(&posX, &data->getData()[104], 4);
	memcpy(&posY, &data->getData()[108], 4);
	memcpy(&posZ, &data->getData()[112], 4);

	float lookAtX = 0.0f, lookAtY = 0.0f, lookAtZ = 0.0f;
	memcpy(&lookAtX, &data->getData()[116], 4);
	memcpy(&lookAtY, &data->getData()[120], 4);
	memcpy(&lookAtZ, &data->getData()[124], 4);

	//128 is '\n'

	string modelPath = string(modelName) + ".obj";
	string texturePath = string(textureName) + ".tga";
	string shaderPath = string(shaderName) + ".glsl";

	new MeshObject(modelPath, texturePath, shaderPath, glm::vec3(posX, posY, posZ), glm::vec3(lookAtX, lookAtY, lookAtZ), glm::vec3(0, 1, 0), netID);

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer
	return 0;
}

DataBuffer * NetworkFunctions::createCreateMeshObjectBuffer(MeshObject * meshObject)
{
	if (meshObject == NULL) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(meshObject->getCreationSize()); //should be 129

	int functionID = EVENT_ADD_MESH_OBJECT;
	tempBuffer->copy(0, &functionID, 4);
	unsigned long netID = meshObject->getNetID();
	tempBuffer->copy(4, &netID, 4);

	string modelName = meshObject->getModelName();
	string textureName = meshObject->getTextureName();
	string shaderName = meshObject->getShaderName();
	string propName = meshObject->getCollisionName();

	char nullBuffer[32]; memset(nullBuffer, '\0', 32); //for (int i = 0; i < 32; i++) nullBuffer[i] = '\0';

	tempBuffer->copy(8, nullBuffer, 32);
	tempBuffer->copy(8, modelName.c_str(), std::min(32, (int)modelName.length()));
	tempBuffer->copy(40, nullBuffer, 32);
	tempBuffer->copy(40, textureName.c_str(), std::min(32, (int)textureName.length()));
	tempBuffer->copy(72, nullBuffer, 32);
	tempBuffer->copy(72, shaderName.c_str(), std::min(32, (int)shaderName.length()));

	glm::vec3 pos = meshObject->getPosition();
	tempBuffer->copy(104, &pos.x, 4);
	tempBuffer->copy(108, &pos.y, 4);
	tempBuffer->copy(112, &pos.z, 4);

	glm::vec3 lookAt = meshObject->getDirection();
	tempBuffer->copy(116, &lookAt.x, 4);
	tempBuffer->copy(120, &lookAt.y, 4);
	tempBuffer->copy(124, &lookAt.z, 4);
	
	tempBuffer->copy(128, "\n", 1);

	return tempBuffer;
}

int NetworkFunctions::spawnPlayer(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 165) return -1;

	unsigned long netID;
	memcpy(&netID, &data->getData()[4], 4);

	int playerID = -1;
	memcpy(&playerID, &data->getData()[8], 4);

	if (playerID < 0 || playerID >= players->size()) return -1;
	if ((*players)[playerID] != NULL) {
		unsigned long netID = players->at(playerID)->getNetID();
		GameObject * tempObject = gameObjects->getValue(netID);
		if (tempObject == NULL) {
			//error
			int x = 123;
		}
		else {
			gameObjects->remove(netID);
			players->at(playerID) = NULL;
			delete tempObject;
		}
	}
		
	char modelName[32];
	//for (int i = 0; i < 32; i++) modelName[i] = '\0';
	memset(modelName, '\0', 32);
	memcpy(modelName, &data->getData()[12], 32);

	char textureName[32];
	//for (int i = 0; i < 32; i++) textureName[i] = '\0';
	memset(textureName, '\0', 32);
	memcpy(textureName, &data->getData()[44], 32);

	char shaderName[32];
	//for (int i = 0; i < 32; i++) shaderName[i] = '\0';
	memset(shaderName, '\0', 32);
	memcpy(shaderName, &data->getData()[76], 32);

	float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
	memcpy(&posX, &data->getData()[108], 4);
	memcpy(&posY, &data->getData()[112], 4);
	memcpy(&posZ, &data->getData()[116], 4);

	float lookAtX = 0.0f, lookAtY = 0.0f, lookAtZ = 0.0f;
	memcpy(&lookAtX, &data->getData()[120], 4);
	memcpy(&lookAtY, &data->getData()[124], 4);
	memcpy(&lookAtZ, &data->getData()[128], 4);

	char propName[33];
	//for (int i = 0; i < 33; i++) propName[i] = '\0';
	memset(propName, '\0', 32);
	memcpy(propName, &data->getData()[132], 32);

	//164 is '\n'

	string modelPath = string(modelName) + ".obj";
	string texturePath = string(textureName) + ".tga";
	string shaderPath = string(shaderName) + ".glsl";
	string propPath = string(propName) + ".prop";

	CharacterObject * newCharacter = NULL;

	if (playerID == clientID)
		newCharacter = new PlayerObject(modelPath, texturePath, shaderPath, glm::vec3(posX, posY, posZ), glm::vec3(lookAtX, lookAtY, lookAtZ), glm::vec3(0, 1, 0), propPath, playerID, netID);
	else
		newCharacter = new CharacterObject(modelPath, texturePath, shaderPath, glm::vec3(posX, posY, posZ), glm::vec3(lookAtX, lookAtY, lookAtZ), glm::vec3(0, 1, 0), propPath, playerID, netID);

	players->at(playerID) = newCharacter;
	
	netID = newCharacter->getNetID();
	data->copy(4, &netID, 4);

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer
	return 0;
}

DataBuffer * NetworkFunctions::createCreateCharacterObjectBuffer(CharacterObject * characterObject)
{
	if (characterObject == NULL) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(characterObject->getCreationSize()); //should be 165

	int functionID = EVENT_SPAWN_PLAYER;
	tempBuffer->copy(0, &functionID, 4);
	unsigned long netID = characterObject->getNetID();
	tempBuffer->copy(4, &netID, 4);
	int playerID = characterObject->getPlayerID();
	tempBuffer->copy(8, &playerID, 4);
	
	string modelName = characterObject->getModelName();
	string textureName = characterObject->getTextureName();
	string shaderName = characterObject->getShaderName();
	string propName = characterObject->getCollisionName();
	
	char nullBuffer[32]; memset(nullBuffer, '\0', 32); //for (int i = 0; i < 32; i++) nullBuffer[i] = '\0';

	tempBuffer->copy(12, nullBuffer, 32);
	tempBuffer->copy(12, modelName.c_str(), std::min(32, (int)modelName.length()));
	tempBuffer->copy(44, nullBuffer, 32);
	tempBuffer->copy(44, textureName.c_str(), std::min(32, (int)textureName.length()));
	tempBuffer->copy(76, nullBuffer, 32);
	tempBuffer->copy(76, shaderName.c_str(), std::min(32, (int)shaderName.length()));

	glm::vec3 pos = characterObject->getPosition();
	tempBuffer->copy(108, &pos.x, 4);
	tempBuffer->copy(112, &pos.y, 4);
	tempBuffer->copy(116, &pos.z, 4);

	glm::vec3 lookAt = characterObject->getDirection();
	tempBuffer->copy(120, &lookAt.x, 4);
	tempBuffer->copy(124, &lookAt.y, 4);
	tempBuffer->copy(128, &lookAt.z, 4);

	tempBuffer->copy(132, nullBuffer, 32);
	tempBuffer->copy(132, propName.c_str(), std::min(32, (int)propName.length()));

	tempBuffer->copy(164, "\n", 1);

	return tempBuffer;
}

DataBuffer * NetworkFunctions::createSpawnPlayerRequestBuffer(int playerID, glm::vec3 pos, glm::vec3 lookAt, int classID)
{
	DataBuffer * tempBuffer = new DataBuffer(165); //should be 165

	int functionID = EVENT_SPAWN_PLAYER;
	tempBuffer->copy(0, &functionID, 4);
	unsigned long netID = 0;
	tempBuffer->copy(4, &netID, 4); //0 for autoassign
	tempBuffer->copy(8, &playerID, 4);

	string modelName = "player"; 
	string textureName = "error";
	string shaderName = "default";
	string propName = "sphere2";

	if (classID == 0) {
		/* Set model, texture, shader, prop by class here */
	}
	
	char nullBuffer[32]; memset(nullBuffer, '\0', 32); //for (int i = 0; i < 32; i++) nullBuffer[i] = '\0';

	tempBuffer->copy(12, nullBuffer, 32);
	tempBuffer->copy(12, modelName.c_str(), std::min(32, (int)modelName.length()));
	tempBuffer->copy(44, nullBuffer, 32);
	tempBuffer->copy(44, textureName.c_str(), std::min(32, (int)textureName.length()));
	tempBuffer->copy(76, nullBuffer, 32);
	tempBuffer->copy(76, shaderName.c_str(), std::min(32, (int)shaderName.length()));

	tempBuffer->copy(108, &pos.x, 4);
	tempBuffer->copy(112, &pos.y, 4);
	tempBuffer->copy(116, &pos.z, 4);

	tempBuffer->copy(120, &lookAt.x, 4);
	tempBuffer->copy(124, &lookAt.y, 4);
	tempBuffer->copy(128, &lookAt.z, 4);

	tempBuffer->copy(132, nullBuffer, 32);
	tempBuffer->copy(132, propName.c_str(), std::min(32, (int)propName.length()));

	tempBuffer->copy(164, "\n", 1);

	return tempBuffer;
}

int NetworkFunctions::killGameObject(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 9) return -1;

	int netID = -1;
	memcpy(&netID, &data->getData()[4], 4);

	GameObject * tempObject = gameObjects->getValue(netID);
	if (tempObject == NULL) return -1;
	tempObject->kill();

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer
	return 0;
}

DataBuffer * NetworkFunctions::createKillObjectBuffer(unsigned long netID)
{
	DataBuffer * tempBuffer = new DataBuffer(9);
	int functionID = EVENT_KILL_GAME_OBJECT;
	tempBuffer->copy(0, &functionID, 4);
	tempBuffer->copy(4, &netID, 4);
	tempBuffer->copy(8, "\n", 1);
	return tempBuffer;
}

DataBuffer * NetworkFunctions::createCreateObjectBuffer(GameObject * gameObject) //int gameObjectIndex)
{
	if (gameObject == NULL) return NULL;

	if (gameObject->getTypeName() == "MeshObject")
		return createCreateMeshObjectBuffer((MeshObject*)gameObject);
	else if (gameObject->getTypeName() == "PropObject")
		return createCreatePropObjectBuffer((PropObject*)gameObject);
	else if (gameObject->getTypeName() == "CharacterObject" || gameObject->getTypeName() == "PlayerObject")
		return createCreateCharacterObjectBuffer((CharacterObject*)gameObject);
	else if (gameObject->getTypeName() == "FullHealthKit")
		return createCreateFullHealthKitObjectBuffer((FullHealthKit*)gameObject);
	else if (gameObject->getTypeName() == "HalfHealthKit")
		return createCreateHalfHealthKitObjectBuffer((HalfHealthKit*)gameObject);
	
	//this shouldnt really ever get called

	return NULL;
}

int NetworkFunctions::deleteGameObject(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 9) return -1;

	unsigned long netID = 0;
	memcpy(&netID, &(data->getData()[4]), 4);

	GameObject * tempObject = gameObjects->getValue(netID);
	if (tempObject == NULL) return -1;

	if ((tempObject->getTypeName() == "CharacterObject" || tempObject->getTypeName() == "PlayerObject")
		&& players->at(((CharacterObject*)tempObject)->getPlayerID()) != NULL) {
		CharacterObject * temp = players->at(((CharacterObject*)tempObject)->getPlayerID());
		players->at(((CharacterObject*)tempObject)->getPlayerID()) = NULL;
		//delete temp;
	}
	//else delete gameObjects->at(objectIndex);
	gameObjects->remove(netID, tempObject);
	delete tempObject;
	
	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer
	return 0;
}

DataBuffer * NetworkFunctions::createDeleteObjectBuffer(unsigned long netID)
{
	DataBuffer * tempBuffer = new DataBuffer(9);
	int functionID = EVENT_DELETE_GAME_OBJECT;
	tempBuffer->copy(0, &functionID, 4);
	tempBuffer->copy(4, &netID, 4);
	tempBuffer->copy(8, "\n", 1);
	return tempBuffer;
}

int NetworkFunctions::updateGameObject(DataBuffer * data)
{
	if (data == NULL) return -1;

	int netID = -1;
	memcpy(&netID, &data->getData()[4], 4);

	GameObject * temp = gameObjects->getValue(netID);
	if (temp != NULL) return temp->deserialize(data); //-1 for error, 0 for success (no echo)
	return -1;
}

DataBuffer * NetworkFunctions::createUpdateObjectBuffer(unsigned long netID)
{
	GameObject * tempObject = gameObjects->getValue(netID);
	if (tempObject == NULL) return NULL;
	DataBuffer * tempBuffer = tempObject->serialize();
	int functionIndex = EVENT_UPDATE_GAME_OBJECT;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &netID, 4);
	return tempBuffer;
}

int NetworkFunctions::addSpell(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 17) return -1;

	int playerID = -1;
	memcpy(&playerID, &data->getData()[4], 4);
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return -1;

	int elemendIndex = -1;
	memcpy(&elemendIndex, &data->getData()[8], 4);
	if (elemendIndex < 0 || elemendIndex > 4) return -1;

	int spellID = -1;
	memcpy(&spellID, &data->getData()[12], 4);
	if (spellID < 0 || spellID > SPELLID_COUNT) return -1;

	if (spellID == SPELLID_FIREBALL) players->at(playerID)->addSpell(new Fireball(playerID, players->at(playerID)), elemendIndex);
	else return -1;

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createAddSpellBuffer(int playerID, int elementIndex, int spellID)
{
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return NULL;
	if (elementIndex < 0 || elementIndex > 4) return NULL;
	if (spellID < 0 || spellID > SPELLID_COUNT) return NULL;

	int functionID = EVENT_ADD_SPELL;

	DataBuffer * tempBuffer = new DataBuffer(17);
	tempBuffer->copy(0, &functionID, 4);
	tempBuffer->copy(4, &playerID, 4);
	tempBuffer->copy(8, &elementIndex, 4);
	tempBuffer->copy(12, &spellID, 4);
	tempBuffer->copy(16, "\n", 1);

	return tempBuffer;
}

//DataBuffer * createAddCustomeSpellBuffer(???)

int NetworkFunctions::changeSpell(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 17) return -1;

	int playerID = -1;
	memcpy(&playerID, &data->getData()[4], 4);
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return -1;
	if (playerID == clientID) return 0; //Ignore commands to change my own spell

	int elementIndex = -1;
	memcpy(&elementIndex, &data->getData()[8], 4);
	if (elementIndex < 0 || elementIndex > 4) return -1;

	int subID = -1;
	memcpy(&subID, &data->getData()[12], 4);
	if (subID < 0 || subID >= players->at(playerID)->getSpells()[elementIndex].size()) return -1;

	//int ret = players->at(playerID)->changeSpell(elementIndex, subID);
	int ret = players->at(playerID)->syncSpell(elementIndex, subID);
	if (ret == 0 && networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer
	return ret;
}

DataBuffer * NetworkFunctions::createChangeSpellBuffer(int playerID, int elementIndex, int subID) //SubID refers to the index in the element vector, IE the Xth fire spell
{
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return NULL;
	if (elementIndex < 0 || elementIndex > 4) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(17);
	int functionIndex = EVENT_CHANGE_SPELL;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &playerID, 4);
	tempBuffer->copy(8, &elementIndex, 4);
	tempBuffer->copy(12, &subID, 4);
	tempBuffer->copy(16, "\n", 1);
	return tempBuffer;
}

int NetworkFunctions::attack(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 37) return -1;

	int playerID = -1;
	memcpy(&playerID, &data->getData()[4], 4);
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return -1;

	int spellID = -1;
	memcpy(&spellID, &data->getData()[8], 4);
	if (spellID < 0 || spellID > SPELLID_COUNT) return -1;

	float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
	memcpy(&posX, &data->getData()[12], 4);
	memcpy(&posY, &data->getData()[16], 4);
	memcpy(&posZ, &data->getData()[20], 4);

	float lookAtX = 0.0f, lookAtY = 0.0f, lookAtZ = 0.0f;
	memcpy(&lookAtX, &data->getData()[24], 4);
	memcpy(&lookAtY, &data->getData()[28], 4);
	memcpy(&lookAtZ, &data->getData()[32], 4);

	//Add shoot effect

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createAttackBuffer(int playerID, int spellID, glm::vec3 pos, glm::vec3 lookAt)
{
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(37);
	int functionIndex = EVENT_ATTACK;
	tempBuffer->copy(0,  &functionIndex, 4);
	tempBuffer->copy(4,	 &playerID, 4);
	tempBuffer->copy(8,	 &spellID, 4);
	tempBuffer->copy(12, &pos.x, 4);
	tempBuffer->copy(16, &pos.y, 4);
	tempBuffer->copy(20, &pos.z, 4);
	tempBuffer->copy(24, &lookAt.x, 4);
	tempBuffer->copy(28, &lookAt.y, 4);
	tempBuffer->copy(32, &lookAt.z, 4);
	tempBuffer->copy(36, "\n", 1);
	return tempBuffer;
}

int NetworkFunctions::damage(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 21) return -1;

	int playerID = -1;
	memcpy(&playerID, &data->getData()[4], 4);
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return -1;

	int targetID = -1;
	memcpy(&targetID, &data->getData()[8], 4);
	if (targetID < 0 || targetID >= players->size() || players->at(targetID) == NULL) return -1;

	int damage = -1;
	memcpy(&damage, &data->getData()[12], 4);

	int spellID = -1;
	memcpy(&spellID, &data->getData()[16], 4);
	if (spellID < 0 || spellID > SPELLID_COUNT) return -1;

	//Damage
	players->at(targetID)->damage(damage, spellID, playerID);

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createDamageBuffer(int playerID, int targetID, int damage, int spellID)
{
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return NULL;
	if (targetID < 0 || targetID >= players->size() || players->at(targetID) == NULL) return NULL;
	if (spellID < 0 || spellID >= SPELLID_COUNT) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(21); 
	int functionIndex = EVENT_DAMAGE;
	tempBuffer->copy(0,  &functionIndex, 4);
	tempBuffer->copy(4,  &playerID, 4);
	tempBuffer->copy(8,  &targetID, 4);
	tempBuffer->copy(12, &damage, 4);
	tempBuffer->copy(16, &spellID, 4);
	tempBuffer->copy(20, "\n", 1);
	return tempBuffer;
}

int NetworkFunctions::heal(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 21) return -1;

	int playerID = -1;
	memcpy(&playerID, &data->getData()[4], 4);

	int targetID = -1;
	memcpy(&targetID, &data->getData()[8], 4);
	if (targetID < 0 || targetID >= players->size() || players->at(targetID) == NULL) return -1;

	int healing = -1;
	memcpy(&healing, &data->getData()[12], 4);

	int spellID = -1;
	memcpy(&spellID, &data->getData()[16], 4);
	if (spellID < 0 || spellID > SPELLID_COUNT) return -1;

	//heal
	players->at(targetID)->heal(healing, spellID, playerID);

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createHealBuffer(int playerID, int targetID, int healing, int spellID)
{
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return NULL;
	if (targetID < 0 || targetID >= players->size() || players->at(targetID) == NULL) return NULL;
	if (spellID < 0 || spellID >= SPELLID_COUNT) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(21);
	int functionIndex = EVENT_HEAL;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &playerID, 4);
	tempBuffer->copy(8, &targetID, 4);
	tempBuffer->copy(12, &healing, 4);
	tempBuffer->copy(16, &spellID, 4);
	tempBuffer->copy(20, "\n", 1);
	return tempBuffer;
}

int NetworkFunctions::force(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 41) return -1;
	
	unsigned long netID = 0;
	memcpy(&netID, &data->getData()[4], 4);

	int playerID = -1;
	memcpy(&playerID, &data->getData()[8], 4);
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return -1;
		
	float worldX, worldY, worldZ;
	memcpy(&worldX, &data->getData()[12], 4);
	memcpy(&worldY, &data->getData()[16], 4);
	memcpy(&worldZ, &data->getData()[20], 4);

	float forceX, forceY, forceZ;
	memcpy(&forceX, &data->getData()[24], 4);
	memcpy(&forceY, &data->getData()[28], 4);
	memcpy(&forceZ, &data->getData()[32], 4);

	int spellID = -1;
	memcpy(&spellID, &data->getData()[36], 4);
	if (spellID < 0 || spellID > SPELLID_COUNT) return -1;

	GameObject * tempObject = gameObjects->getValue(netID);
	if (tempObject == NULL) return -1;
	tempObject->applyImpulse(worldX, worldY, worldZ, forceX, forceY, forceZ);
	//gameObjects->at(gameObjectIndex)->applyImpulse(worldX, worldY, worldZ, forceX, forceY, forceZ);

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createForceBuffer(int playerID, unsigned long netID, glm::vec3 worldPos, glm::vec3 force, int spellID)
{
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return NULL;
	if (spellID < 0 || spellID >= SPELLID_COUNT) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(41);
	int functionIndex = EVENT_FORCE;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &netID, 4);
	tempBuffer->copy(8, &playerID, 4);
	tempBuffer->copy(12, &worldPos.x, 4);
	tempBuffer->copy(16, &worldPos.y, 4);
	tempBuffer->copy(20, &worldPos.z, 4);
	tempBuffer->copy(24, &force.x, 4);
	tempBuffer->copy(28, &force.y, 4);
	tempBuffer->copy(32, &force.z, 4);
	tempBuffer->copy(36, &spellID, 4);
	tempBuffer->copy(40, "\n", 1);
	return tempBuffer;
}

int NetworkFunctions::centralForce(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 29) return -1;

	unsigned long netID = 0;
	memcpy(&netID, &data->getData()[4], 4);

	int playerID = -1;
	memcpy(&playerID, &data->getData()[8], 4);
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return -1;

	float forceX, forceY, forceZ;
	memcpy(&forceX, &data->getData()[12], 4);
	memcpy(&forceY, &data->getData()[16], 4);
	memcpy(&forceZ, &data->getData()[20], 4);

	int spellID = -1;
	memcpy(&spellID, &data->getData()[24], 4);
	if (spellID < 0 || spellID > SPELLID_COUNT) return -1;

	GameObject * tempObject = gameObjects->getValue(netID);
	if (tempObject == NULL) return -1;
	tempObject->applyCentralImpulse(forceX, forceY, forceZ);
	//gameObjects->at(gameObjectIndex)->applyCentralImpulse(forceX, forceY, forceZ);

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createCentralForceBuffer(int playerID, unsigned long netID, glm::vec3 force, int spellID)
{
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return NULL;
	if (spellID < 0 || spellID >= SPELLID_COUNT) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(29);
	int functionIndex = EVENT_FORCE;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &netID, 4);
	tempBuffer->copy(8, &playerID, 4);
	tempBuffer->copy(12, &force.x, 4);
	tempBuffer->copy(16, &force.y, 4);
	tempBuffer->copy(20, &force.z, 4);
	tempBuffer->copy(24, &spellID, 4);
	tempBuffer->copy(28, "\n", 1);
	return tempBuffer;
}

int NetworkFunctions::spawnFullHealthKit(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 21) return -1;

	unsigned long netID = 0;
	memcpy(&netID, &data->getData()[4], 4);

	float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
	memcpy(&posX, &data->getData()[8], 4);
	memcpy(&posY, &data->getData()[12], 4);
	memcpy(&posZ, &data->getData()[16], 4);

	new FullHealthKit(glm::vec3(posX, posY, posZ), netID);

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createSpawnFullHealthKitBuffer(glm::vec3 position, unsigned long netID)
{
	DataBuffer * tempBuffer = new DataBuffer(21);
	int functionIndex = EVENT_SPAWN_FULL_HEALTH_KIT;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &netID, 4);
	tempBuffer->copy(8,  &position.x, 4);
	tempBuffer->copy(12,  &position.y, 4);
	tempBuffer->copy(16, &position.z, 4);
	tempBuffer->copy(20, "\n", 1);
	return tempBuffer;
}

DataBuffer * NetworkFunctions::createCreateFullHealthKitObjectBuffer(FullHealthKit * fullHealthKit)
{
	if (fullHealthKit == NULL) return NULL;
	return createSpawnFullHealthKitBuffer(fullHealthKit->getPosition(), fullHealthKit->getNetID());
}

int NetworkFunctions::spawnHalfHealthKit(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 21) return -1;

	unsigned long netID = 0;
	memcpy(&netID, &data->getData()[4], 4);

	float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
	memcpy(&posX, &data->getData()[8], 4);
	memcpy(&posY, &data->getData()[12], 4);
	memcpy(&posZ, &data->getData()[16], 4);

	new HalfHealthKit(glm::vec3(posX, posY, posZ), netID);

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createSpawnHalfHealthKitBuffer(glm::vec3 position, unsigned long netID)
{
	DataBuffer * tempBuffer = new DataBuffer(17);
	int functionIndex = EVENT_SPAWN_HALF_HEALTH_KIT;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &netID, 4);
	tempBuffer->copy(8, &position.x, 4);
	tempBuffer->copy(12, &position.y, 4);
	tempBuffer->copy(16, &position.z, 4);
	tempBuffer->copy(20, "\n", 1);
	return tempBuffer;
}

DataBuffer * NetworkFunctions::createCreateHalfHealthKitObjectBuffer(HalfHealthKit * halfHealthKit)
{
	if (halfHealthKit == NULL) return NULL;
	return createSpawnHalfHealthKitBuffer(halfHealthKit->getPosition(), halfHealthKit->getNetID());
}

int NetworkFunctions::createFireball(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 33) return -1;

	int playerID = -1;
	memcpy(&playerID, &data->getData()[4], 4);
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return -1;
	
	float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
	memcpy(&posX, &data->getData()[8], 4);
	memcpy(&posY, &data->getData()[12], 4);
	memcpy(&posZ, &data->getData()[16], 4);

	float lookAtX = 0.0f, lookAtY = 0.0f, lookAtZ = 0.0f;
	memcpy(&lookAtX, &data->getData()[20], 4);
	memcpy(&lookAtY, &data->getData()[24], 4);
	memcpy(&lookAtZ, &data->getData()[28], 4);

	glm::vec3 pos = glm::vec3(posX, posY, posZ);
	glm::vec3 lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
	lookAt = lookAt == glm::vec3(0, 0, 0) ? glm::vec3(0, 0, 1) : glm::normalize(lookAt);

	FireballProjectile * temp = new FireballProjectile("fireballsmall.obj", "error.tga", "default.glsl",
		pos + lookAt*0.5f, lookAt, glm::vec3(0,1,0), "sphere1.prop", players->at(playerID));

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createCreateFireballBuffer(int playerID, glm::vec3 pos, glm::vec3 dir)
{
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(33);
	int functionIndex = EVENT_CREATE_FIREBALL;
	tempBuffer->copy(0,  &functionIndex, 4);
	tempBuffer->copy(4,  &playerID, 4);
	tempBuffer->copy(8,  &pos.x, 4);
	tempBuffer->copy(12, &pos.y, 4);
	tempBuffer->copy(16, &pos.z, 4);
	tempBuffer->copy(20, &dir.x, 4);
	tempBuffer->copy(24, &dir.y, 4);
	tempBuffer->copy(28, &dir.z, 4);
	tempBuffer->copy(32, "\n", 1);

	return tempBuffer;
}

int NetworkFunctions::createGrenade(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 33) return -1;

	int playerID = -1;
	memcpy(&playerID, &data->getData()[4], 4);
	if (playerID < 0 || playerID >= players->size() || players->at(playerID) == NULL) return -1;

	float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
	memcpy(&posX, &data->getData()[8], 4);
	memcpy(&posY, &data->getData()[12], 4);
	memcpy(&posZ, &data->getData()[16], 4);

	float lookAtX = 0.0f, lookAtY = 0.0f, lookAtZ = 0.0f;
	memcpy(&lookAtX, &data->getData()[20], 4);
	memcpy(&lookAtY, &data->getData()[24], 4);
	memcpy(&lookAtZ, &data->getData()[28], 4);

	glm::vec3 pos = glm::vec3(posX, posY, posZ);
	glm::vec3 lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
	lookAt = lookAt == glm::vec3(0, 0, 0) ? glm::vec3(0, 0, 1) : glm::normalize(lookAt);

	new Grenade("sphere.obj", "green.tga", "default.glsl",
		pos + lookAt*0.5f, lookAt, glm::vec3(0, 1, 0), "sphere1.prop", players->at(playerID));

	if (networkState == NETWORK_STATE_SERVER) eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createCreateGrenadeBuffer(int playerID, glm::vec3 pos, glm::vec3 dir)
{
	if (playerID < 0 || playerID >= (int)players->size() || players->at(playerID) == NULL) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(33);
	int functionIndex = EVENT_CREATE_GRENADE;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &playerID, 4);
	tempBuffer->copy(8, &pos.x, 4);
	tempBuffer->copy(12, &pos.y, 4);
	tempBuffer->copy(16, &pos.z, 4);
	tempBuffer->copy(20, &dir.x, 4);
	tempBuffer->copy(24, &dir.y, 4);
	tempBuffer->copy(28, &dir.z, 4);
	tempBuffer->copy(32, "\n", 1);

	return tempBuffer;
}

int NetworkFunctions::createFadingBeamEffect(DataBuffer * data)
{
	if (data == NULL || data->getSize() != 45) return -1;

	float startX = 0.0f, startY = 0.0f, startZ = 0.0f;
	memcpy(&startX, &data->getData()[4], 4);
	memcpy(&startY, &data->getData()[8], 4);
	memcpy(&startZ, &data->getData()[12], 4);

	float endX = 0.0f, endY = 0.0f, endZ = 0.0f;
	memcpy(&endX, &data->getData()[16], 4);
	memcpy(&endY, &data->getData()[20], 4);
	memcpy(&endZ, &data->getData()[24], 4);

	float colourR = 0.0f, colourG = 0.0f, colourB = 0.0f;
	memcpy(&colourR, &data->getData()[28], 4);
	memcpy(&colourG, &data->getData()[32], 4);
	memcpy(&colourB, &data->getData()[36], 4);

	long duration = 0;
	memcpy(&duration, &data->getData()[40], 4);

	if (networkState != NETWORK_STATE_SERVER) new FadingBeamEffect(glm::vec3(startX, startY, startZ), glm::vec3(endX, endY, endZ), glm::vec3(colourR, colourG, colourB), duration);
	else eventPacketTCP->appendToSendBuffer(data->getData(), data->getSize()); //copy to send buffer

	return 0;
}

DataBuffer * NetworkFunctions::createCreateFadingBeamEffectBuffer(glm::vec3 start, glm::vec3 end, glm::vec3 colour, long duration)
{
	DataBuffer * tempBuffer = new DataBuffer(45);
	int functionIndex = EVENT_CREATE_FADING_BEAM_EFFECT;
	tempBuffer->copy(0,  &functionIndex, 4);
	tempBuffer->copy(4,  &start.x, 4);
	tempBuffer->copy(8,  &start.y, 4);
	tempBuffer->copy(12, &start.z, 4);
	tempBuffer->copy(16, &end.x, 4);
	tempBuffer->copy(20, &end.y, 4);
	tempBuffer->copy(24, &end.z, 4);
	tempBuffer->copy(28, &colour.x, 4);
	tempBuffer->copy(32, &colour.y, 4);
	tempBuffer->copy(36, &colour.z, 4);
	tempBuffer->copy(40, &duration, 4);
	tempBuffer->copy(44, "\n", 1);

	return tempBuffer;
}
