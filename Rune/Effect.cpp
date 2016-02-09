#include "Effect.h"

list<Effect*> * Effect::effectList = NULL;

Effect::Effect() 
{ 
	alive = true;
	effectList->push_back(this);
}
Effect::~Effect() {}
