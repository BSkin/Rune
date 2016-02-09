#include "PercentHealthPickupObject.h"

PercentHealthPickupObject::PercentHealthPickupObject(glm::vec3 position, unsigned long netID) : PickupObject("rune.obj", "purple.tga", "default.glsl", position, "sphere1.prop", netID)
{
	healPercent = 0.0f;
}

PercentHealthPickupObject::~PercentHealthPickupObject() {}

void PercentHealthPickupObject::collisionCallback(RigidObject * other)
{
	int networkState = getNetworkState();
	if (networkState == NETWORK_STATE_OFFLINE || networkState == NETWORK_STATE_SERVER) {
		if (other == NULL || alive == false) return;
		if (other->getTypeName() == "CharacterObject" || other->getTypeName() == "PlayerObject") {
			if (((CharacterObject *)other)->getHealth() < ((CharacterObject *)other)->getMaxHealth()) {
				((CharacterObject *)other)->healPercentage(healPercent);
				alive = false;
			}
		}
	}
}