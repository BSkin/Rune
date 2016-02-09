#ifndef GRENADE_H
#define GRENADE_H

#include "PropObject.h"

class Grenade : public PropObject
{
public:
	Grenade(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, /*list<Grenade *> * ownerGrenadeList,*/ void * owner = 0); //ok for now
	virtual ~Grenade();

	virtual void collisionCallback(RigidObject * other);
	void * getOwner() { return owner; }
protected:
	virtual void explode();
	virtual int update(long elapsedTime);
	btRigidBody * initExplosionBody();
	void * owner;
	float force, damage;
	long startTime, duration;
	bool airborne;

	//list<Grenade *> * ownerGrenadeList;
};

#endif