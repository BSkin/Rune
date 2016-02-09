#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "RenderObject.h"

#include "LinkedHashMap.h"
#include "DataBuffer.h"

class GameObject : public RenderObject
{
public:
	GameObject(unsigned long netID = 0);
	GameObject(string, string, string, unsigned long netID = 0); //ok for now
	GameObject(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, unsigned long netID = 0); //ok for now
	virtual ~GameObject();

	bool isAlive() { return alive; }
	virtual void kill() { alive = false; }
	
	virtual int update(long elapsedTime);
	virtual int render(long totalElapsedTime);

	virtual int damage(int x, int sourceID = 0, int attackerID = -1) { return 0; }
	virtual void applyCentralImpulse(glm::vec3 x) {}
	virtual void applyCentralImpulse(float x, float y, float z) {}
	virtual void applyImpulse(float worldX, float worldY, float worldZ, float forceX, float forceY, float forceZ) {}
	virtual void applyImpulse(glm::vec3 worldPos, glm::vec3 force) {}
	virtual void applyImpulse(btVector3 &worldPos, btVector3 &force) {}
	
	virtual DataBuffer * createSpellForceBuffer(float worldX, float worldY, float worldZ, float forceX, float forceY, float forceZ, int spellID) { return createSpellForceBuffer(glm::vec3(worldX, worldY, worldZ), glm::vec3(forceX, forceY, forceZ), spellID); }
	virtual DataBuffer * createSpellForceBuffer(glm::vec3 worldPos, glm::vec3 force, int spellID);
	virtual DataBuffer * createSpellCentralForceBuffer(float forceX, float forceY, float forceZ, int spellID) { return createSpellCentralForceBuffer(glm::vec3(forceX, forceY, forceZ), spellID); }
	virtual DataBuffer * createSpellCentralForceBuffer(glm::vec3 force, int spellID);
	virtual DataBuffer * serialize();
	virtual int deserialize(DataBuffer * data);
	virtual int getSerializedSize() { return 33; }
	virtual int getCreationSize() { return 125; }
	
	virtual string getTypeName() { return "GameObject"; }

	unsigned long getNetID() { return netID; }

	static void resetNextNetID() { nextNetID = 1; }
	static void setStatics(LinkedHashMap<unsigned long, GameObject *> * objectVector) { gameObjects = objectVector; }
protected:
	bool alive;
private:
	void init(unsigned long netID);
	unsigned long netID;
	static unsigned long nextNetID;

	static LinkedHashMap<unsigned long, GameObject *> * gameObjects;
};

#endif