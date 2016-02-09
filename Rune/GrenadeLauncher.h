#ifndef GRENADE_LAUNCHER_H
#define GRENADE_LAUNCHER_H

#include "Spell.h"
#include "Grenade.h"

class GrenadeLauncher : public Spell
{
public:
	GrenadeLauncher(int ownerID, void * owner);
	~GrenadeLauncher();

	virtual void primaryClick();
	//virtual void secondaryClick();

	//list<Grenade*> * getGrenadeListAddress() { return &grenadeList; }
private:
	DataBuffer * createCreateGrenadeBuffer();
	//list<Grenade*> grenadeList;
};

#endif