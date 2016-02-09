#ifndef RAGDOLLOBJECT_H
#define RAGDOLLOBJECT_H

#include "PhysicsObject.h"

#define PI 3.14159

class RagdollObject : public PhysicsObject
{
public:
	RagdollObject();
	RagdollObject(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up);	
	~RagdollObject();

	virtual int render(long totalElapsedTime);
	virtual int update(long elapsedTime);

	virtual void setVelocity(glm::vec3 vel) { bodies[0]->setLinearVelocity(btVector3(vel.x, vel.y, vel.z)); }
	virtual void setVelocity(float x, float y, float z) { bodies[0]->setLinearVelocity(btVector3(x, y, z)); }
	virtual void setGravity(glm::vec3 grav) { for (int i = 0; i < 14; i++) bodies[i]->setGravity(btVector3(grav.x, grav.y, grav.z)); }
	virtual void setGravity(float x, float y, float z) { for (int i = 0; i < 14; i++) bodies[i]->setGravity(btVector3(x, y, z)); }

protected:
	virtual int initBullet();

	glm::vec3 * scalings;
	btCollisionShape ** colShapes;
	btRigidBody ** bodies;
};

#endif