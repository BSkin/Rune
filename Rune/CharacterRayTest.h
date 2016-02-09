#ifndef CHARACTER_RAY_TEST
#define CHARACTER_RAY_TEST

#include "Bullet\src\btBulletDynamicsCommon.h"
#include "RigidObject.h"

class CharacterRayTest : public btCollisionWorld::ClosestRayResultCallback
{
public:
	CharacterRayTest(const btVector3& rayFromWorld, const btVector3& rayToWorld, void * owner) : btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld) { this->owner = owner; }
      
private:
	void * owner;
	virtual bool needsCollision(btBroadphaseProxy* proxy0) const
	{
		const btCollisionObject* co = (btCollisionObject*)proxy0->m_clientObject;
                 
		//if ((co->getOwner() != NULL) || (co->getOwner() != owner) || (((PhysicsObject*)co->getOwner())->getCollisionType() == "projectile"))
		if (co->getOwner() == owner || (co->getOwner() != NULL && (((PhysicsObject*)co->getOwner())->getCollisionType() == "projectile")))
				return false;
         
		return btCollisionWorld::ClosestRayResultCallback::needsCollision(proxy0);
	}
};

#endif