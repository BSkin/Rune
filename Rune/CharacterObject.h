#ifndef CHARACTER_OBJECT_H
#define CHARACTER_OBJECT_H

#include "glm/gtx/rotate_vector.hpp"

#include "PropObject.h"
#include "Spell.h"
#include "Fireball.h"
#include "GrenadeLauncher.h"
#include "Railgun.h"
#include "Shotgun.h"
#include "AirborneContactCallback.h"

#define maxSpeed 100.0f

class Spell;

class CharacterObject : public PropObject
{
public:
	CharacterObject(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, int playerID, unsigned long netID = 0); //ok for now
	virtual ~CharacterObject();

	virtual int update(long elapsedTime);
	virtual int render(long totalElapsedTime);

	virtual glm::vec3 getDirection() { return GameObject::getDirection(); }
	int getPlayerID() { return playerID; }

	virtual DataBuffer * serialize();
	virtual int deserialize(DataBuffer * data);
	virtual int getSerializedSize() { return 53; }

	virtual string getTypeName() { return "CharacterObject"; }
	virtual int getCreationSize() { return 165; }

	bool getAir() { return getAirborne(); }
	glm::vec2 getAngle() { return directionAngle;  }

	void addSpell(Spell * spell, int elementIndex);
	int changeSpell(int elementIndex, int subIndex = 0);
	int syncSpell(int elementIndex, int subIndex = 0);
	vector<Spell*> * getSpells() { return spells; } //size 5
	Spell * getSpell(int elementIndex, int subIndex);
	void getActiveSpellIndices(int * outElementIndex, int * outSubIndex);

	DataBuffer * createChangeSpellBuffer(int elementIndex, int subID);

	int getHealth() { return health; }
	int getMaxHealth() { return maxHealth; }
	virtual int setMaxHealth(int x);
	virtual int getMaxHealth(int x);
	virtual int damage(int x, int sourceID = SPELLID_WORLD, int attackerID = -1);
	virtual int heal(int x, int sourceID = SPELLID_WORLD, int healerID = -1);
	virtual int healPercentage(float x, int sourceID = SPELLID_WORLD, int healerID = -1);
protected:
	void runForward();
	void runBackward();
	void runLeft();
	void runRight();

	void walkForward();
	void walkBackward();
	void walkLeft();
	void walkRight();

	void turn(float x, float y);
	void turn(POINT x) { turn((float)x.x, (float)x.y); }
	void turnLeft(float angle);
	void turnRight(float angle);
	void turnUp(float angle);
	void turnDown(float angle);
	
	void setDirection(glm::vec3 direction);
	void setDirection(glm::vec3 direction, glm::vec3 left, glm::vec3 up);

	void primaryClick()		{ if (activeSpell == NULL) return; activeSpell->primaryClick(); }
	void primaryDown()		{ if (activeSpell == NULL) return; activeSpell->primaryDown(); }
	void primaryUp()		{ if (activeSpell == NULL) return; activeSpell->primaryUp(); }
	void primaryRelease()	{ if (activeSpell == NULL) return; activeSpell->primaryRelease(); }

	void secondaryClick()	{ if (activeSpell == NULL) return; activeSpell->secondaryClick(); }
	void secondaryDown()	{ if (activeSpell == NULL) return; activeSpell->secondaryClick(); }
	void secondaryUp()		{ if (activeSpell == NULL) return; activeSpell->secondaryClick(); }
	void secondaryRelease() { if (activeSpell == NULL) return; activeSpell->secondaryClick(); }

	void jump();
	void cycleSpell(int elementIndex);
private:
	void walk(float x, float y, float z, long elapsedTime);
	void walk(glm::vec3 val, long elapsedTime) { walk(val.x, val.y, val.z, elapsedTime); }
	void airwalk(float x, float y, float z, long elapsedTime);
	void airwalk(glm::vec3 val, long elapsedTime) { airwalk(val.x, val.y, val.z, elapsedTime); }
	void AirControl(glm::vec3 wishdir, float wishspeed, long elapsedTime);
	void Accelerate(glm::vec3 wishdir, float wishspeed, float accel, long elapsedTime);

	void stopWalking(long elapsedTime);
	bool getAirborne() { return airborne; }
	void testGround();

	DataBuffer * createDamageBuffer(int damage, int sourceID, int attackerID);
	DataBuffer * createHealBuffer(int healing, int sourceID, int attackerID);
	
	int playerID;

	Spell * activeSpell;
	vector<Spell*> spells[5]; //0 melee, 1 fire, 2 water, 3 earth, 4 air
	int currentElementIndex, currentSpellIndex;

	btRigidBody * airborneTest;
	btRigidBody * createAirborneTest();

	RigidObject * vehicle;
	btVector3 vehicleSpeed;
	long noVehicleTimer;

	int health, maxHealth;
	bool airborne;
	bool sliding;
	float floorDist;
	btVector3 groundPos;
	btVector3 groundNormal;
	glm::vec2 frameTurnAmount;
	glm::vec2 directionAngle;
	glm::vec3 left;
	glm::vec3 walkDirection;
	glm::vec2 localWalkDirection;

	static float moveSpeed;  // Ground move speed
	static float runAcceleration;   // Ground accel
	static float runDeacceleration;   // Deacceleration that occurs when running on the ground
	static float airAcceleration;  // Air accel
	static float airDeacceleration;  // Deacceleration experienced when opposite strafing
	static float airControl;  // How precise air control is
	static float sideStrafeAcceleration;   // How fast acceleration occurs to get up to sideStrafeSpeed when side strafing
	static float sideStrafeSpeed;    // What the max speed to generate when side strafing
	static float jumpSpeed;  // The speed at which the character's up axis gains when hitting jump
	static float moveScale;
};

#endif