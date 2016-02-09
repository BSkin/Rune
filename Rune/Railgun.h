#ifndef RAILGUN_H
#define RAILGUN_H

#include "Spell.h"
#include "FadingBeamEffect.h"
#include "CharacterRayTest.h"
#include "Bullet\src\btBulletDynamicsCommon.h"

#define MAX_DISTANCE 10000.0f
#define BEAM_DURATION 1000

class Railgun : public Spell
{
public:
	Railgun(int ownerID, void * owner);
	~Railgun();

	virtual void primaryClick();
};

#endif RAILGUN_H