#include "PickupObject.h"

PickupObject::PickupObject(string m, string t, string s, glm::vec3 position, string collisionPath, unsigned long netID) : PropObject(m, t, s, position, glm::vec3(0, 0, 1), glm::vec3(0,1,0), collisionPath, netID)
{
	if (physInit) {
		body->setLinearFactor(btVector3(0, 0, 0));
		body->setAngularFactor(0);
		body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	}
}

PickupObject::~PickupObject() {}