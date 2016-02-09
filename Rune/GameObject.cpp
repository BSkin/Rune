#include "GameObject.h"

LinkedHashMap<unsigned long, GameObject *> * GameObject::gameObjects = NULL;
unsigned long GameObject::nextNetID = 1;

void GameObject::init(unsigned long netID)
{
	if (netID == 0) this->netID = nextNetID++;
	else {
		this->netID = netID;
		if (netID >= nextNetID) nextNetID = netID + 1;
	}
	alive = true;
	gameObjects->add(this->netID, this);
}

GameObject::GameObject(unsigned long netID)
{
	init(netID);
}

GameObject::GameObject(string mPath, string tPath, string sPath, unsigned long netID) : RenderObject(mPath, tPath, sPath)
{
	init(netID);
}

GameObject::GameObject(string mPath, string tPath, string sPath, glm::vec3 position, glm::vec3 direction, glm::vec3 up, unsigned long netID) : RenderObject(mPath, tPath, sPath, position, direction, up)
{
	init(netID);
}

GameObject::~GameObject()
{
	//vector<GameObject*>::iterator position = std::find(gameObjectVector->begin(), gameObjectVector->end(), this);
	//gameObjectVector->erase(position);
	//gameObjectVector->remove(this);
}

/*
functionID	4
objectIndex	4
posX		4
posY		4
posZ		4
lookAtX		4
lookAtY		4
lookAtZ		4*/

DataBuffer * GameObject::createSpellForceBuffer(glm::vec3 worldPos, glm::vec3 force, int spellID)
{
	int playerID = -1;
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

DataBuffer * GameObject::createSpellCentralForceBuffer(glm::vec3 force, int spellID)
{
	int playerID = -1;
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

DataBuffer * GameObject::serialize()
{
	DataBuffer * tempBuffer = new DataBuffer(getSerializedSize());

	int uninit = -1;
	tempBuffer->copy(0, &uninit, 4);

	tempBuffer->copy(4,  &netID, 4);
	tempBuffer->copy(8,	 &position.x, 4);
	tempBuffer->copy(12, &position.y, 4);
	tempBuffer->copy(16, &position.z, 4);
	tempBuffer->copy(20, &direction.x, 4);
	tempBuffer->copy(24, &direction.y, 4);
	tempBuffer->copy(28, &direction.z, 4);

	tempBuffer->copy(32, "\n", 1);

	return tempBuffer;
}

int GameObject::deserialize(DataBuffer * data)
{
	if (data == NULL || data->getSize() != getSerializedSize()) return -1;

	float floatData[6];
	memcpy(&floatData[0],	&data->getData()[8],	4);
	memcpy(&floatData[1],	&data->getData()[12],	4);
	memcpy(&floatData[2],	&data->getData()[16],	4);
	memcpy(&floatData[3],	&data->getData()[20],	4);
	memcpy(&floatData[4],	&data->getData()[24],	4);
	memcpy(&floatData[5],	&data->getData()[28],	4);

	position = glm::vec3(floatData[0], floatData[1], floatData[2]);
	direction = glm::vec3(floatData[3], floatData[4], floatData[5]);

	return 0;
}

int GameObject::update(long elapsedTime)
{
	return RenderObject::update(elapsedTime);
}

int GameObject::render(long totalElapsedTime)
{
	return RenderObject::render(totalElapsedTime);
}