#ifndef AIRBORNE_CONTACT_CALLBACK_H
#define AIRBORNE_CONTACT_CALLBACK_H

#include "Bullet\src\btBulletDynamicsCommon.h"
#include "Utilities.h"
#include <string>
#include "RigidObject.h"

using std::string;
using namespace Utilities;

struct AirborneContactCallback : public btCollisionWorld::ContactResultCallback 
{
	btRigidBody& body; //!< The body the sensor is monitoring
	RigidObject ** vehicle;
    string ctxt; //!< External information for contact processing
	bool * airborne;
	btVector3 * groundNormal;

    AirborneContactCallback(btRigidBody& tgtBody , string context, bool * airborne, btVector3 * groundNormal, RigidObject ** vehicle /*, ... */)
        : btCollisionWorld::ContactResultCallback(), body(tgtBody), ctxt(context) 
	{ this->airborne = airborne; this->groundNormal = groundNormal; this->vehicle = vehicle; }

	virtual bool needsCollision(btBroadphaseProxy* proxy) const {
		if(!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
            return false;
        return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
    }

	virtual btScalar addSingleResult(btManifoldPoint& cp,
        const btCollisionObjectWrapper* colObj0,int partId0,int index0,
        const btCollisionObjectWrapper* colObj1,int partId1,int index1)
    {
		if (airborne == NULL) return 0;
	
		if (((btRigidBody*)colObj0->getCollisionObject())->getOwner() != 
 			((btRigidBody*)colObj1->getCollisionObject())->getOwner()) {
			if (&body == (btRigidBody*)colObj0->getCollisionObject() && cp.m_localPointA.y() < -0.707 && ((PhysicsObject * )(btRigidBody*)colObj1->getCollisionObject()->getOwner())->getCollisionType() != "projectile") {
				if (((PhysicsObject * )(btRigidBody*)colObj1->getCollisionObject()->getOwner())->getCollisionType() == "vehicle")
					*vehicle = ((RigidObject * )(btRigidBody*)colObj1->getCollisionObject()->getOwner());
				*airborne = false;
				*groundNormal = cp.m_normalWorldOnB;
			}
			else if (&body == (btRigidBody*)colObj1->getCollisionObject() && cp.m_localPointB.y() < -0.707 && ((PhysicsObject * )(btRigidBody*)colObj0->getCollisionObject()->getOwner())->getCollisionType() != "projectile") {
				if (((PhysicsObject * )(btRigidBody*)colObj0->getCollisionObject()->getOwner())->getCollisionType() == "vehicle")
					*vehicle = ((RigidObject * )(btRigidBody*)colObj0->getCollisionObject()->getOwner());
				*airborne = false;
				*groundNormal = cp.m_normalWorldOnB;
			}
		}

		return 0;
    }
};

#endif