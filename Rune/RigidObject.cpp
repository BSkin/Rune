#include "RigidObject.h"

RigidObject::RigidObject() : PhysicsObject() {}
RigidObject::RigidObject(string m, string t, string s) : PhysicsObject(m, t, s) {}
RigidObject::RigidObject(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, unsigned long netID, int colGroup, int colMask) : PhysicsObject(m, t, s, position, direction, up, netID) 
{
	collisionName = path;
	collisionGroup = colGroup;
	collisionMask = colMask;
}
RigidObject::~RigidObject() 
{ 
	if (!physInit) return;

	dynamicsWorld->removeCollisionObject( body );
	delete body->getMotionState();
	delete body; 
}

int RigidObject::initBullet(string path, int collisionGroup, int collisionMask)
{
	//Collision Shape already created by child class
	
	/// Create Dynamic Objects
	btTransform startTransform;
	startTransform.setIdentity();

	btScalar mass = ((path == "model" || path == "heightField") ? 0.0f : 1.0f);
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass,localInertia);
		
	/*startTransform.setOrigin(btVector3(
					btScalar(position.x),
					btScalar(position.y),
					btScalar(position.z)));*/

	worldMatrix = glm::inverse(glm::lookAt(position, position-direction, up));
	startTransform.setFromOpenGLMatrix(glm::value_ptr(worldMatrix));

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	body = new btRigidBody(mass,myMotionState,colShape,localInertia, this);

	dynamicsWorld->addRigidBody(body, collisionGroup, collisionMask);
	body->setFriction(0.5f);

	//body->setActivationState(ISLAND_SLEEPING);	
	//body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

	return PhysicsObject::initBullet();
}

/*
functionID	4
objectIndex	4
posX		4
posY		4
posZ		4
linVelX		4
linVelY		4
linVelZ		4
orientW		4
orientX		4
orientY		4
orientZ		4
angVelX		4
angVelY		4
angVelZ		4*/

DataBuffer * RigidObject::serialize()
{
	if (!physInit) return NULL;

	btVector3 pos = body->getCenterOfMassPosition();
	btVector3 linVel = body->getLinearVelocity();
	btQuaternion orient = body->getOrientation();
	btVector3 angVel = body->getAngularVelocity();

	DataBuffer * tempBuffer = new DataBuffer(getSerializedSize());

	float test = 1.0f;
	memcpy(&test, &pos.x(), 4);

	int uninit = -1;
	tempBuffer->copy(0, &uninit, 4); //function ID
	
	unsigned long netID = getNetID();
	tempBuffer->copy(4, &netID, 4);
	tempBuffer->copy(8,		&pos.x(),		4);
	tempBuffer->copy(12,	&pos.y(),		4);
	tempBuffer->copy(16,	&pos.z(),		4);
	tempBuffer->copy(20,	&linVel.x(),	4);
	tempBuffer->copy(24,	&linVel.y(),	4);
	tempBuffer->copy(28,	&linVel.z(),	4);
	tempBuffer->copy(32,	&orient.x(),	4);
	tempBuffer->copy(36,	&orient.y(),	4);
	tempBuffer->copy(40,	&orient.z(),	4);
	tempBuffer->copy(44,	&orient.w(),	4);
	tempBuffer->copy(48,	&angVel.x(),	4);
	tempBuffer->copy(52,	&angVel.y(),	4);
	tempBuffer->copy(56,	&angVel.z(),	4);

	tempBuffer->copy(60, "\n", 1);

	return tempBuffer;
}

int RigidObject::deserialize(DataBuffer * data)
{
	if (!physInit || data == NULL || data->getSize() != getSerializedSize()) return -1;

	btVector3 pos = body->getCenterOfMassPosition();
	btVector3 linVel = body->getLinearVelocity();
	btQuaternion orient = body->getOrientation();
	btVector3 angVel = body->getAngularVelocity();

	float floatData[13];
	memcpy(&floatData[0],	&data->getData()[8],	4);
	memcpy(&floatData[1],	&data->getData()[12],	4);
	memcpy(&floatData[2],	&data->getData()[16],	4);
	memcpy(&floatData[3],	&data->getData()[20],	4);
	memcpy(&floatData[4],	&data->getData()[24],	4);
	memcpy(&floatData[5],	&data->getData()[28],	4);
	memcpy(&floatData[6],	&data->getData()[32],	4);
	memcpy(&floatData[7],	&data->getData()[36],	4);
	memcpy(&floatData[8],	&data->getData()[40],	4);
	memcpy(&floatData[9],	&data->getData()[44],	4);
	memcpy(&floatData[10],	&data->getData()[48],	4);
	memcpy(&floatData[11],	&data->getData()[52],	4);
	memcpy(&floatData[12],	&data->getData()[56],	4);

	btTransform newTransform = btTransform(btQuaternion(floatData[6], floatData[7], floatData[8], floatData[9]), btVector3(floatData[0], floatData[1], floatData[2]));
	body->setWorldTransform(newTransform);
	body->setLinearVelocity(btVector3(floatData[3], floatData[4], floatData[5]));
	body->setAngularVelocity(btVector3(floatData[10], floatData[11], floatData[12]));

	return 0;
}

int RigidObject::render(long totalElapsedTime)
{
	if (!physInit) return -1;
	body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(worldMatrix)); // * scale
	normalMatrix = glm::transpose(glm::inverse(worldMatrix));

	return GameObject::render(totalElapsedTime);
}

int RigidObject::update(long elapsedTime)
{
	PhysicsObject::update(elapsedTime);
	if (!physInit) initBullet(collisionName, collisionGroup, collisionMask);
	if (!physInit) return -1;
	
	return 0;
}

void RigidObject::setRotation(glm::vec3 lookAt, glm::vec3 up) 
{
	direction = lookAt;
	this->up = up;
	btVector3 posBT = body->getCenterOfMassPosition(); 
	glm::vec3 posGLM = glm::vec3(posBT.x(), posBT.y(), posBT.z());
	worldMatrix = glm::inverse(glm::lookAt(posGLM, posGLM+lookAt, up));

	btTransform trans;	
	trans.setFromOpenGLMatrix(glm::value_ptr(worldMatrix));
	body->setWorldTransform(trans);
	body->activate(true);
}

void RigidObject::setPosition(float x, float y, float z)
{
	btTransform trans = body->getWorldTransform();
	trans.setOrigin(btVector3(x, y, z));
	body->setWorldTransform(trans);
	body->activate(true);
}

glm::vec3 RigidObject::getDirection()
{
	if (!physInit) return GameObject::getDirection();

	btScalar matrix[16];
	body->getWorldTransform().getOpenGLMatrix(matrix);
	return glm::vec3(matrix[8], matrix[9], matrix[10]);
}

void RigidObject::setMass(float x)
{
	if (!physInit) return;

	//dynamicsWorld->removeRigidBody( body );
	btVector3 inertia;
	body->getCollisionShape()->calculateLocalInertia( x, inertia );
	body->setMassProps(x, inertia);
	//dynamicsWorld->addRigidBody( body );
}