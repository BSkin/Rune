#ifndef FIREBALL_PROJECTILE_H
#define FIREBALL_PROJECTILE_H

#include "PropObject.h"

class FireballProjectile : public PropObject
{
public:
	FireballProjectile(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, void * owner = 0); //ok for now
	virtual ~FireballProjectile();

	virtual void collisionCallback(RigidObject * other);
	void * getOwner() { return owner; }
protected:
	virtual void explode();
	btRigidBody * initExplosionBody();
	void * owner;
	float force, damage;
};

#endif