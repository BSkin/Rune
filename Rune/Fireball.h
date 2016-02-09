#ifndef FIREBALL_H
#define FIREBALL_H

#include "Spell.h"

class Fireball : public Spell
{
public:
	Fireball(int ownerID, void * owner);
	~Fireball();

	virtual void primaryClick();
private:
	DataBuffer * createCreateFireballBuffer();

};

#endif