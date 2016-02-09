#ifndef PICKUP_OBJECT_H
#define PICKUP_OBJECT_H

#include "PropObject.h"
#include "CharacterObject.h"

class PickupObject : public PropObject
{
public:
	PickupObject(string m, string t, string s, glm::vec3 position, string collisionPath, unsigned long netID = 0);
	virtual ~PickupObject();

	virtual void collisionCallback(RigidObject * other) {}
	virtual string getTypeName() { return "PickupObject"; }
};

#endif