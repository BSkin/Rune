#ifndef BUOYANCY_POINT_H
#define BUOYANCY_POINT_H

#include "Bullet\src\btBulletDynamicsCommon.h"
#include "Bullet\src\BulletCollision\CollisionShapes\btBvhTriangleMeshShape.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"	

class BuoyancyPoint
{
public:
	BuoyancyPoint(float x, float y, float z);
	BuoyancyPoint(glm::vec3 pos);
	btVector3 update(const btTransform * parentTransform);
	btVector3 update(btTransform * parentTransform) { return update((const btTransform *)parentTransform); }

	btVector3 getLocalPosition();
private:
	btTransform localTransformation, parentTransform;
	btVector3 localPosition;

	void init(float x, float y, float z);
};

#endif