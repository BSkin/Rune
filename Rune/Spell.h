#ifndef SPELL_H
#define SPELL_H

#include "RenderObject.h"
#include "GameObject.h"

#define SPELLID_WORLD		0
#define SPELLID_FIREBALL	1
#define SPELLID_RAILGUN		2
#define SPELLID_SHOTGUN		3
#define SPELLID_GRENADE		4

#define SPELLID_COUNT 5

class Spell : public RenderObject
{
public:
	Spell(string, string, string, int ownerID, void * owner); //ok for now
	//string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, GameObject * owner); //ok for now
	virtual ~Spell();

	virtual void primaryClick() {}
	virtual void primaryDown() {}
	virtual void primaryUp() {}
	virtual void primaryRelease() {}

	virtual void secondaryClick() {}
	virtual void secondaryDown() {}
	virtual void secondaryUp() {}
	virtual void secondaryRelease() {}

	virtual int updateSpell(glm::vec3 position, glm::vec3 direction, glm::vec3 up, long elapsedTime);
	virtual int render(long totalElapsedTime);
	
	int getSpellID() { return spellID; }
	static string getSpellName(int i) { if (i < 0 || i >= SPELLID_COUNT) return "Error"; return spellNames[i]; }
protected:
	void * owner;
	int ownerID;
	int ammunition;
	int spellID;

	//DataBuffer * createAttackBuffer();
	DataBuffer * createDamageBuffer(int playerID, int targetID, int damage, int spellID);
	DataBuffer * createForceBuffer(GameObject * g, glm::vec3 worldPos, glm::vec3 force, int spellID);
	DataBuffer * createForceBuffer(GameObject * g, float worldX, float worldY, float worldZ, float forceX, float forceY, float forceZ, int spellID) { return createForceBuffer(g, glm::vec3(worldX, worldY, worldZ), glm::vec3(forceX, forceY, forceZ), spellID); }
	DataBuffer * createCentralForceBuffer(GameObject * g, glm::vec3 force, int spellID);
	DataBuffer * createCentralForceBuffer(GameObject * g, float forceX, float forceY, float forceZ, int spellID) { return createCentralForceBuffer(g, glm::vec3(forceX, forceY, forceZ), spellID); }
	DataBuffer * createCreateFadingBeamEffectBuffer(glm::vec3 start, glm::vec3 end, glm::vec3 colour, long duration);
private:
	void init();

	static string spellNames[];
};

#endif SPELL_H