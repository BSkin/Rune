#include "GrenadeLauncher.h"

GrenadeLauncher::GrenadeLauncher(int ownerID, void * owner) : Spell("fireballlauncher.obj", "green.tga", "default.glsl", ownerID, owner)
{
	spellID = SPELLID_GRENADE;

	//grenadeList = list<Grenade*>();
}

GrenadeLauncher::~GrenadeLauncher()
{
	/*while (grenadeList.size() > 0) {
		delete grenadeList.back();
		grenadeList.pop_back();
	}*/
}

DataBuffer * GrenadeLauncher::createCreateGrenadeBuffer()
{
	if (owner == NULL) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(33);
	int functionIndex = EVENT_CREATE_GRENADE;
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

void GrenadeLauncher::primaryClick()
{
	if (getNetworkState() == NETWORK_STATE_CLIENT) {
		addTCPEvent(createCreateGrenadeBuffer());
	}
	else {
		new Grenade("sphere.obj", "green.tga", "default.glsl", position + direction*0.5f, direction, up, "sphere1.prop", /*&grenadeList,*/ owner);
		if (getNetworkState() == NETWORK_STATE_SERVER)
			addTCPEvent(createCreateGrenadeBuffer());
	}
}

/*void GrenadeLauncher::secondaryClick()
{
	if (grenadeList.size() == 0) return;

}*/