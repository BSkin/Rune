#ifndef FULL_HEALTH_KIT_H
#define FULL_HEALTH_KIT_H

#include "PercentHealthPickupObject.h"

class FullHealthKit : public PercentHealthPickupObject
{
public:
	FullHealthKit(glm::vec3 position, unsigned long netID = 0) : PercentHealthPickupObject(position, netID) { healPercent = 1.0f; }
	virtual ~FullHealthKit() {}

	virtual string getTypeName() { return "FullHealthKit"; }
};

#endif