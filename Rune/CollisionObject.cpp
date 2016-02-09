#include "CollisionObject.h"

CollisionObject::CollisionObject() : RigidObject() {}
CollisionObject::CollisionObject(string m, string t, string s) : RigidObject(m, t, s) {} //ok for now
CollisionObject::CollisionObject(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path) : RigidObject(m, t, s, position, direction, up, path, COL_NONE, COL_NONE) 
{
	initBullet(path);
}

CollisionObject::~CollisionObject() 
{
	dynamicsWorld->removeCollisionObject(ghost);
	delete ghost;
}

int CollisionObject::initBullet(string path)
{
	ghost = new btPairCachingGhostObject(this);
	ghost->setWorldTransform(body->getWorldTransform());
	ghost->setCollisionShape (PhysicsObject::getCollisionShape("sphere10"));
	if (ghost->getCollisionShape() == NULL) return -1;

	ghost->setCollisionFlags(ghost->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	body->setCollisionFlags(body->getCollisionFlags());// | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	//ghost->setActivationState(DISABLE_DEACTIVATION);
	//body->setActivationState(DISABLE_DEACTIVATION); 
	dynamicsWorld->addCollisionObject(ghost, COL_DEFAULT, COL_DEFAULT);

	return 0;
}

int CollisionObject::update(long elapsedTime)
{
	RigidObject::update(elapsedTime);
	ghost->setWorldTransform(body->getWorldTransform());
	collide();
	return 0;
};

int CollisionObject::render(long totalElapsedTime)
{
	return RigidObject::render(totalElapsedTime);
}

void CollisionObject::collide()
{
	
} 

void CollisionObject::collisionCallback(RigidObject * otherObject)
{
	
}