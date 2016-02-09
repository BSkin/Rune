#ifndef EFFECT_H
#define EFFECT_H

#include <list>
using std::list;

class Effect
{
public:
	Effect();
	virtual ~Effect();

	virtual int update(long elapsedTime) { return 0; }
	virtual int render(long totalElapsedTime) { return 0; }
	bool isAlive() { return alive; }

	static void setStatics(list<Effect*> * x) { effectList = x; }

protected:
	bool alive;

private:
	static list<Effect*> * effectList;
};

#endif