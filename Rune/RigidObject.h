#ifndef RIGIDOBJECT_H
#define RIGIDOBJECT_H

#include "PhysicsObject.h"

class RigidObject : public PhysicsObject
{
public:
	RigidObject();
	RigidObject(string, string, string); //ok for now
	RigidObject(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, unsigned long netID = 0, int colGroup = COL_DEFAULT, int colMask = COL_DEFAULT); //ok for now
	virtual ~RigidObject() = 0;

	virtual int render(long totalElapsedTime);
	virtual int update(long elapsedTime);

	btVector3 getWorldPosFromLocalPos(float x, float y, float z) { if (!physInit) return btVector3(0,0,0); return body->getWorldTransform() * btVector3(x, y, z); }
	btVector3 getLocalPosFromWorldPos(float x, float y, float z) { if (!physInit) return btVector3(0,0,0); return body->getWorldTransform().inverse() * btVector3(x, y, z); }
	btVector3 getLocalPosFromWorldPos(glm::vec3 vec)			{ return getLocalPosFromWorldPos(vec.x, vec.y, vec.z); }

	virtual btVector3 getAngularVelocity()						{ if (!physInit) return btVector3(0,0,0); return body->getAngularVelocity(); }
	virtual btVector3 getVelocity()								{ if (!physInit) return btVector3(0,0,0); return body->getLinearVelocity(); }
	virtual void setAngularVelocity(float x, float y, float z)	{ if (!physInit) return; body->setAngularVelocity(btVector3(x, y, z)); }
	virtual void setAngularVelocity(glm::vec3 vel)				{ setAngularVelocity(vel.x, vel.y, vel.z); }
	virtual void setVelocity(float x, float y, float z)			{ if (!physInit) return; body->setLinearVelocity(btVector3(x, y, z)); }
	virtual void setVelocity(glm::vec3 vel)						{ setVelocity(vel.x, vel.y, vel.z); }
	virtual btQuaternion getOrientation()						{ if (!physInit) return btQuaternion(0, 0, 0); return body->getOrientation(); }
	virtual glm::vec3 getDirection();

	virtual void setGravity(glm::vec3 grav)						{ if (!physInit) return; body->setGravity(btVector3(grav.x, grav.y, grav.z)); body->activate(true); }
	virtual void setGravity(float x, float y, float z)			{ if (!physInit) return; body->setGravity(btVector3(x, y, z)); body->activate(true); }
	virtual void setFriction(float x)							{ if (!physInit) return; body->setFriction(x); }
	virtual void setMass(float x);
	virtual void setRotation(glm::vec3 lookAt, glm::vec3 up);

	virtual void applyCentralImpulse(glm::vec3 x)				{ if (!physInit) return; body->applyCentralImpulse(Utilities::glmToBT3(x)); }
	virtual void applyCentralImpulse(float x, float y, float z)	{ applyCentralImpulse(glm::vec3(x, y, z)); }
	
	virtual void applyImpulse(btVector3 &worldPos, btVector3 &force) { if (!physInit) return; body->activate(true); body->applyImpulse(force, worldPos - getBTPosition()); }
	virtual void applyImpulse(glm::vec3 worldPos, glm::vec3 force) { return applyImpulse(Utilities::glmToBT3(worldPos), Utilities::glmToBT3(force)); }
	virtual void applyImpulse(float worldX, float worldY, float worldZ, float forceX, float forceY, float forceZ) { return applyImpulse(btVector3(worldX, worldY, worldZ), btVector3(forceX, forceY, forceZ)); }

	virtual glm::vec3 getPosition()								{ if (!physInit) return position; return btToGLM3(&body->getCenterOfMassPosition()); }
	virtual btVector3 getBTPosition()							{ if (!physInit) return btVector3(0,0,0); return body->getCenterOfMassPosition(); }
	virtual void setPosition(float x, float y, float z);
	virtual void wakeUp()										{ if (!physInit) return; body->activate(true); }

	virtual DataBuffer * serialize();
	virtual int deserialize(DataBuffer * data);
	virtual int getSerializedSize() { return 61; }
	virtual int getCharID() { return -1; }

	virtual string getTypeName() { return "RigidObject"; }
	string getCollisionPath() { return "Collision Shapes\\" + collisionName; }
	string getCollisionName() { return collisionName.substr(0, collisionName.length()-5); }

	virtual void collisionCallback(RigidObject * other) {}
protected:
	int initBullet(string path, int colGroup, int colMask);
	int collisionGroup, collisionMask;
	string collisionName;

	//btTriangleMesh *mTriMesh;// = new btTriangleMesh();
	btRigidBody * body;
};

#endif