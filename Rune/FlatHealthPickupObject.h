#ifndef FLAT_HEALTH_PICKUP_OBJECT_H
#define FLAT_HEALTH_PICKUP_OBJECT_H

#include "PickupObject.h"

class FlatHealthPickupObject : public PickupObject
{
public:
	FlatHealthPickupObject(glm::vec3 position, unsigned long netID = 0);
	virtual ~FlatHealthPickupObject();

	virtual void collisionCallback(RigidObject * other);
	virtual string getTypeName() { return "HealthPickupObject"; }
protected:
	int healAmount;
};

#endif