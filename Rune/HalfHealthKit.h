#ifndef HALF_HEALTH_KIT_H
#define HALF_HEALTH_KIT_H

#include "PercentHealthPickupObject.h"

class HalfHealthKit : public PercentHealthPickupObject
{
public:
	HalfHealthKit(glm::vec3 position, unsigned long netID = 0) : PercentHealthPickupObject(position, netID) { healPercent = 0.5f; }
	virtual ~HalfHealthKit() {}

	virtual string getTypeName() { return "HalfHealthKit"; }
};

#endif