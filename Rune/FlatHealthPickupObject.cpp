#include "FlatHealthPickupObject.h"

FlatHealthPickupObject::FlatHealthPickupObject(glm::vec3 position, unsigned long netID) : PickupObject("rune.obj", "purple.tga", "default.glsl", position, "sphere1.prop", netID)
{
	healAmount = 0;
}

FlatHealthPickupObject::~FlatHealthPickupObject() {}

void FlatHealthPickupObject::collisionCallback(RigidObject * other)
{
	int networkState = getNetworkState();
	if (networkState == NETWORK_STATE_OFFLINE || networkState == NETWORK_STATE_SERVER) {
		if (other == NULL || alive == false) return;
		if (other->getTypeName() == "CharacterObject" || other->getTypeName() == "PlayerObject") {
			if (((CharacterObject *)other)->getHealth() < ((CharacterObject *)other)->getMaxHealth()) {
				((CharacterObject *)other)->heal(healAmount);
				alive = false;
			}
		}
	}
}