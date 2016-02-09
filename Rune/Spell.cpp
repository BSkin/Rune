#include "Spell.h"

string Spell::spellNames[] = {
	"World",		//0
	"Fireball",		//1
	"Railgun",		//2
	"Shotgun",		//3
};

Spell::Spell(string mPath, string tPath, string sPath, int ownerID, void * owner) : RenderObject(mPath, tPath, sPath)//ok for now
{
	init();
	this->ownerID = ownerID;
	this->owner = owner;
}

Spell::~Spell() {}

void Spell::init()
{
	spellID = SPELLID_WORLD;
	ammunition = 0;
}

int Spell::updateSpell(glm::vec3 position, glm::vec3 direction, glm::vec3 up, long elapsedTime)
{
	this->position = position;
	this->direction = direction;
	this->up = up;
	RenderObject::updateMatrices();
	return RenderObject::update(elapsedTime);
}

int Spell::render(long totalElapsedTime)
{
	return RenderObject::render(totalElapsedTime);
}

DataBuffer * Spell::createDamageBuffer(int playerID, int targetID, int damage, int spellID)
{
	if (spellID < 0 || spellID >= SPELLID_COUNT) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(21);
	int functionIndex = EVENT_DAMAGE;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &playerID, 4);
	tempBuffer->copy(8, &targetID, 4);
	tempBuffer->copy(12, &damage, 4);
	tempBuffer->copy(16, &spellID, 4);
	tempBuffer->copy(20, "\n", 1);
	return tempBuffer;
}


DataBuffer * Spell::createForceBuffer(GameObject * g, glm::vec3 worldPos, glm::vec3 force, int spellID)
{
	if (spellID < 0 || spellID >= SPELLID_COUNT) return NULL;
	if (g == NULL) return NULL;

	DataBuffer * tempBuffer = g->createSpellForceBuffer(worldPos, force, spellID);
	tempBuffer->copy(8, &ownerID, 4);
	return tempBuffer;
}

DataBuffer * Spell::createCentralForceBuffer(GameObject * g, glm::vec3 force, int spellID)
{
	if (spellID < 0 || spellID >= SPELLID_COUNT) return NULL;
	if (g == NULL) return NULL;

	DataBuffer * tempBuffer = g->createSpellCentralForceBuffer(force, spellID);
	tempBuffer->copy(8, &ownerID, 4);
	return tempBuffer;
}

DataBuffer * Spell::createCreateFadingBeamEffectBuffer(glm::vec3 start, glm::vec3 end, glm::vec3 colour, long duration)
{
	DataBuffer * tempBuffer = new DataBuffer(45);
	int functionIndex = EVENT_CREATE_FADING_BEAM_EFFECT;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &start.x, 4);
	tempBuffer->copy(8, &start.y, 4);
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