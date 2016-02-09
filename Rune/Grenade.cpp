#include "Grenade.h"
#include "ExplosionContactCallback.h"

Grenade::Grenade(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, /*list<Grenade*> * ownerGrenadeList,*/ void * owner) : PropObject(m, t, s, position, direction, up, path)
{
	this->owner = owner;
	force = 2.5f;
	damage = 15.0f;
	collisionType = "projectile";
	//body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

	setVelocity(direction*50.0f);
	setRotation(direction, up);
	body->setAngularFactor(0.1f);
	body->setRestitution(0.0f);
	body->setFriction(2.0f);

	startTime = GetCurrentTime();
	duration = 3000;
	airborne = true;
	//if (this->ownerGrenadeList != NULL) this->ownerGrenadeList->push_back(this);
}
Grenade::~Grenade()
{
	//if (ownerGrenadeList != NULL) ownerGrenadeList->remove(this);
}

btRigidBody * Grenade::initExplosionBody()
{
	btRigidBody * temp;
	btCollisionShape * colShape = PhysicsObject::getCollisionShape("rocketExplosion");

	/// Create Dynamic Objects
	btTransform startTransform;
	startTransform.setIdentity();

	btScalar mass = 1.0f;
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass, localInertia);

	startTransform.setOrigin(btVector3(body->getWorldTransform().getOrigin()));

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	temp = new btRigidBody(mass, myMotionState, colShape, localInertia, owner);
	delete myMotionState;
	return temp;
}

int Grenade::update(long elapsedTime)
{
	if ((startTime + duration) < GetCurrentTime()) {
		explode();
		alive = false;
	}

	return PropObject::update(elapsedTime);
}

void Grenade::explode()
{
	if (!alive) return;
	btRigidBody * tgtBody = initExplosionBody();
	string foo = "asdf";
	ExplosionContactCallback callback(*tgtBody, foo, force, damage, getNetworkState());
	dynamicsWorld->updateAabbs();
	dynamicsWorld->contactTest(tgtBody, callback);
	delete tgtBody;
}

void Grenade::collisionCallback(RigidObject * otherObject)
{
	if (otherObject == owner || !airborne) return;
	if (!(otherObject->getTypeName() == "CharacterObject" || otherObject->getTypeName() == "PlayerObject")) {
		airborne = false;
		return;
	}
	explode();
	alive = false;
}