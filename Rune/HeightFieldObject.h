#ifndef HEIGHT_FIELD_OBJECT_H
#define HEIGHT_FIELD_OBJECT_H

#include "RigidObject.h"
#include "Bullet\src\BulletCollision\CollisionShapes\btHeightfieldTerrainShape.h"

class HeightFieldObject : public RigidObject
{
public:
	HeightFieldObject(string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, float * heightField, int width, int length, float offsetY, int colGroup = COL_DEFAULT, int colMask = COL_DEFAULT); //ok for now
	virtual ~HeightFieldObject();
	virtual string getTypeName() { return "HeightFieldObject"; }
protected:
	int initBullet(float * heightField, int width, int height, float offsetY, int colGroup, int colMask);
	float * heightData;
};

#endif