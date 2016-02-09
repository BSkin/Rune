#ifndef PROP_OBJECT_H
#define PROP_OBJECT_H

#include "RigidObject.h"

class PropObject : public RigidObject
{
public:
	PropObject(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, unsigned long netID = 0, int colGroup = COL_DEFAULT, int colMask = COL_DEFAULT);
	virtual ~PropObject();
	virtual string getTypeName() { return "PropObject"; }
	virtual int getCreationSize() { return 161; }
protected:
	int initBullet(string path,int colGroup, int colMask);
};

#endif