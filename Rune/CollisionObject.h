#ifndef COLLISIONOBJECT_H
#define COLLISIONOBJECT_H

#include "RigidObject.h"
#include "BulletCollision\CollisionDispatch\btGhostObject.h"

class CollisionObject : public RigidObject
{
public:
	CollisionObject();
	CollisionObject(string, string, string); //ok for now
	CollisionObject(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path); //ok for now
	virtual ~CollisionObject();

	virtual int render(long elapsedTime);
	virtual int update(long totalElapsedTime);

	virtual void collisionCallback(RigidObject * other);
protected:
	virtual int initBullet(string path);

	virtual void collide();
	btPairCachingGhostObject * ghost;
private:

};

#endif