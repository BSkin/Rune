#ifndef SHOTGUN_H
#define SHOTGUN_H

#include "Spell.h"
#include "FadingBeamEffect.h"
#include "CharacterRayTest.h"
#include "Bullet\src\btBulletDynamicsCommon.h"

#define MAX_DISTANCE 1000.0f
#define BEAM_DURATION 250
#define PELLET_FORCE 1.0f

struct PelletHitData {
	int playerID;
	RigidObject * hitObject;
	btVector3 end;
};

class Shotgun : public Spell
{
public:
	Shotgun(int ownerID, void * owner);
	~Shotgun();

	virtual void primaryClick();
private:
	PelletHitData * getHitData(btVector3 &start, btVector3 &end);
};

#endif