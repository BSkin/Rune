#ifndef PERCENT_HEALTH_PICKUP_OBJECT_H
#define PERCENT_HEALTH_PICKUP_OBJECT_H

#include "PickupObject.h"

class PercentHealthPickupObject : public PickupObject
{
public:
	PercentHealthPickupObject(glm::vec3 position, unsigned long netID = 0);
	virtual ~PercentHealthPickupObject();

	virtual void collisionCallback(RigidObject * other);
	virtual string getTypeName() { return "HealthPickupObject"; }
protected:
	float healPercent;
};

#endif