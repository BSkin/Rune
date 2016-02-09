#include "Fireball.h"
#include "FireballProjectile.h"

Fireball::Fireball(int ownerID, void * owner) : Spell("fireballlauncher.obj", "FireballLauncher.tga", "default.glsl", ownerID, owner)
{
	spellID = SPELLID_FIREBALL;
}

Fireball::~Fireball()
{

}

DataBuffer * Fireball::createCreateFireballBuffer()
{
	if (owner == NULL) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(33);
	int functionIndex = EVENT_CREATE_FIREBALL;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &ownerID, 4);
	tempBuffer->copy(8, &position.x, 4);
	tempBuffer->copy(12, &position.y, 4);
	tempBuffer->copy(16, &position.z, 4);
	tempBuffer->copy(20, &direction.x, 4);
	tempBuffer->copy(24, &direction.y, 4);
	tempBuffer->copy(28, &direction.z, 4);
	tempBuffer->copy(32, "\n", 1);

	return tempBuffer;
}

void Fireball::primaryClick()
{
	if (getNetworkState() == NETWORK_STATE_CLIENT) {
		addTCPEvent(createCreateFireballBuffer());
	}
	else {
		FireballProjectile * temp = new FireballProjectile("fireballsmall.obj", "error.tga", "default.glsl", position + direction*0.5f, direction, up, "sphere1.prop", owner);
		if (getNetworkState() == NETWORK_STATE_SERVER)
			addTCPEvent(createCreateFireballBuffer());
	}
} 