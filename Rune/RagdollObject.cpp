#include "RagdollObject.h"

struct Bone {
	float headX, headY, headZ;
	float tailX, tailY, tailZ;
	float radius;
	short id;
	Bone * parent;
};

RagdollObject::RagdollObject()
{

}

RagdollObject::RagdollObject(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up) : PhysicsObject(m, t, s, position, direction, up)
{
	initBullet();
}

RagdollObject::~RagdollObject()
{

}

int RagdollObject::initBullet()
{
	colShapes = new btCollisionShape*[14];
	bodies = new btRigidBody*[14];
	scalings = new glm::vec3[14];
	//torso, uleg. lleg, foot, uleg, lleg, foot, uarm, larm, hand, uarm, larm, hand, head

	scalings[0] = glm::vec3(1.0,2.0,0.5);
	scalings[1] = glm::vec3(0.5,2.0,0.5);
	scalings[2] = glm::vec3(0.5,2.0,0.5);
	scalings[3] = glm::vec3(0.5,0.5,1.0);
	scalings[4] = glm::vec3(0.5,2.0,0.5);
	scalings[5] = glm::vec3(0.5,2.0,0.5);
	scalings[6] = glm::vec3(0.5,0.5,1.0);
	scalings[7] = glm::vec3(2.0,0.5,0.5);
	scalings[8] = glm::vec3(2.0,0.5,0.5);
	scalings[9] = glm::vec3(1.0,0.5,0.5);
	scalings[10] = glm::vec3(2.0,0.5,0.5);
	scalings[11] = glm::vec3(2.0,0.5,0.5);
	scalings[12] = glm::vec3(1.0,0.5,0.5);
	scalings[13] = glm::vec3(1.0,1.0,1.0);

	colShapes[0] = new btBoxShape(btVector3(scalings[0].x/2,scalings[0].y/2,scalings[0].z/2));
	collisionShapes->push_back(colShapes[0]);
	colShapes[1] = new btBoxShape(btVector3(scalings[1].x/2,scalings[1].y/2,scalings[1].z/2));
	collisionShapes->push_back(colShapes[1]);
	colShapes[2] = new btBoxShape(btVector3(scalings[2].x/2,scalings[2].y/2,scalings[2].z/2));
	collisionShapes->push_back(colShapes[2]);
	colShapes[3] = new btBoxShape(btVector3(scalings[3].x/2,scalings[3].y/2,scalings[3].z/2));
	collisionShapes->push_back(colShapes[3]);
	colShapes[4] = new btBoxShape(btVector3(scalings[4].x/2,scalings[4].y/2,scalings[4].z/2));
	collisionShapes->push_back(colShapes[4]);
	colShapes[5] = new btBoxShape(btVector3(scalings[5].x/2,scalings[5].y/2,scalings[5].z/2));
	collisionShapes->push_back(colShapes[5]);
	colShapes[6] = new btBoxShape(btVector3(scalings[6].x/2,scalings[6].y/2,scalings[6].z/2));
	collisionShapes->push_back(colShapes[6]);
	colShapes[7] = new btBoxShape(btVector3(scalings[7].x/2,scalings[7].y/2,scalings[7].z/2));
	collisionShapes->push_back(colShapes[7]);
	colShapes[8] = new btBoxShape(btVector3(scalings[8].x/2,scalings[8].y/2,scalings[8].z/2));
	collisionShapes->push_back(colShapes[8]);
	colShapes[9] = new btBoxShape(btVector3(scalings[9].x/2,scalings[9].y/2,scalings[9].z/2));
	collisionShapes->push_back(colShapes[9]);
	colShapes[10] = new btBoxShape(btVector3(scalings[10].x/2,scalings[10].y/2,scalings[10].z/2));
	collisionShapes->push_back(colShapes[10]);
	colShapes[11] = new btBoxShape(btVector3(scalings[11].x/2,scalings[11].y/2,scalings[11].z/2));
	collisionShapes->push_back(colShapes[11]);
	colShapes[12] = new btBoxShape(btVector3(scalings[12].x/2,scalings[12].y/2,scalings[12].z/2));
	collisionShapes->push_back(colShapes[12]);
	colShapes[13] = new btBoxShape(btVector3(scalings[13].x/2,scalings[13].y/2,scalings[13].z/2));
	collisionShapes->push_back(colShapes[13]);

	btTransform startTransform;
	startTransform.setIdentity();
	btScalar mass(1.f);
	btVector3 localInertia(0,0,0);

	mass = 3.0f;
	colShapes[0]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x),	btScalar(position.y),	btScalar(position.z)));
	btDefaultMotionState* myMotionState0 = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState0,colShapes[0],localInertia);
	bodies[0] = new btRigidBody(rbInfo);

	mass = 2.0f;
	colShapes[1]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x-0.5),	btScalar(position.y-2.0),	btScalar(position.z)));
	btDefaultMotionState* myMotionState1 = new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState1,colShapes[1],localInertia);
	bodies[1] = new btRigidBody(rbInfo);

	mass = 2.0f;
	colShapes[2]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x-0.5),	btScalar(position.y-4.0),	btScalar(position.z)));
	btDefaultMotionState* myMotionState2 = new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState2,colShapes[2],localInertia);
	bodies[2] = new btRigidBody(rbInfo);

	mass = 1.0f;
	colShapes[3]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x-0.5),	btScalar(position.y-5.25),	btScalar(position.z+0.5)));
	btDefaultMotionState* myMotionState3 = new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState3,colShapes[3],localInertia);
	bodies[3] = new btRigidBody(rbInfo);

	mass = 2.0f;
	colShapes[4]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x+0.5),	btScalar(position.y-2.0),	btScalar(position.z)));
	btDefaultMotionState* myMotionState4 = new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState4,colShapes[4],localInertia);
	bodies[4] = new btRigidBody(rbInfo);

	mass = 2.0f;
	colShapes[5]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x+0.5),	btScalar(position.y-4.0),	btScalar(position.z)));
	btDefaultMotionState* myMotionState5 = new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState5,colShapes[5],localInertia);
	bodies[5] = new btRigidBody(rbInfo);

	mass = 1.0f;
	colShapes[6]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x+0.5),	btScalar(position.y-5.25),	btScalar(position.z+0.5)));
	btDefaultMotionState* myMotionState6 = new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState6,colShapes[6],localInertia);
	bodies[6] = new btRigidBody(rbInfo);

	mass = 2.0f;
	colShapes[7]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x-1.5),	btScalar(position.y+1),	btScalar(position.z)));
	btDefaultMotionState* myMotionState7 = new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState7,colShapes[7],localInertia);
	bodies[7] = new btRigidBody(rbInfo);

	mass = 2.0f;
	colShapes[8]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x-3.5),	btScalar(position.y+1),	btScalar(position.z)));
	btDefaultMotionState* myMotionState8= new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState8,colShapes[8],localInertia);
	bodies[8] = new btRigidBody(rbInfo);

	mass = 1.0f;
	colShapes[9]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x-5),	btScalar(position.y+1),	btScalar(position.z)));
	btDefaultMotionState* myMotionState9= new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState9,colShapes[9],localInertia);
	bodies[9] = new btRigidBody(rbInfo);

	mass = 2.0f;
	colShapes[10]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x+1.5),	btScalar(position.y+1),	btScalar(position.z)));
	btDefaultMotionState* myMotionState10 = new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState10,colShapes[10],localInertia);
	bodies[10] = new btRigidBody(rbInfo);

	mass = 2.0f;
	colShapes[11]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x+3.5),	btScalar(position.y+1),	btScalar(position.z)));
	btDefaultMotionState* myMotionState11 = new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState11,colShapes[11],localInertia);
	bodies[11] = new btRigidBody(rbInfo);

	mass = 1.0f;
	colShapes[12]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x+5),	btScalar(position.y+1),	btScalar(position.z)));
	btDefaultMotionState* myMotionState12= new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState12,colShapes[12],localInertia);
	bodies[12] = new btRigidBody(rbInfo);

	mass = 1.0f;
	colShapes[13]->calculateLocalInertia(mass,localInertia);
	startTransform.setOrigin(btVector3(	btScalar(position.x),	btScalar(position.y+2),	btScalar(position.z)));
	btDefaultMotionState* myMotionState13= new btDefaultMotionState(startTransform);
	rbInfo = btRigidBody::btRigidBodyConstructionInfo(mass,myMotionState13,colShapes[13],localInertia);
	bodies[13] = new btRigidBody(rbInfo);
	
	//torso and head
	btTransform localA; localA.setOrigin(btVector3(0.0f,1.0f,0.0f));
	btTransform localB; localB.setOrigin(btVector3(0.0f,-0.6f,0.0f));
	btConeTwistConstraint * neckConstraint = new btConeTwistConstraint( *bodies[0], *bodies[13], localA, localB );
	neckConstraint->setLimit((float)M_PI/4.0f, (float)M_PI/4.0f, (float)M_PI/2.0f);
	dynamicsWorld->addConstraint( neckConstraint );

	//torso and lleg
	localA.setOrigin(btVector3(-0.5,-1,0));
	localB.setOrigin(btVector3(0,1.5,0));
	btConeTwistConstraint * lHipConstraint = new btConeTwistConstraint( *bodies[0], *bodies[1], localA, localB );
	lHipConstraint->setLimit((float)M_PI/4.0f, (float)M_PI/4.0f, 0.0f);
	dynamicsWorld->addConstraint( lHipConstraint );

	//ulleg and lleg
	btHingeConstraint * lKneeConstraint = new btHingeConstraint( *bodies[1], *bodies[2], btVector3(0,-1.25,0), btVector3(0,1.25,0), btVector3(1,0,0), btVector3(1,0,0));
	lKneeConstraint->setLimit(0.0f, (float)M_PI*0.9f);
	dynamicsWorld->addConstraint( lKneeConstraint );

	//ulleg and lfoot
	localA.setOrigin(btVector3(0.0f,-1.0f,0.0f));
	localB.setOrigin(btVector3(0.0f,0.5f,0.5f));
	btConeTwistConstraint * lAnkleConstraint = new btConeTwistConstraint( *bodies[2], *bodies[3], localA, localB );
	lAnkleConstraint->setLimit((float)M_PI/4.0f, (float)M_PI/4.0f, 0.0f);
	dynamicsWorld->addConstraint( lAnkleConstraint );

	//torso and rleg
	localA.setOrigin(btVector3(0.5f,-1.0f,0.0f));
	localB.setOrigin(btVector3(0.0f,1.5f,0.0f));
	btConeTwistConstraint * rHipConstraint = new btConeTwistConstraint( *bodies[0], *bodies[4], localA, localB );
	rHipConstraint->setLimit((float)M_PI/4.0f, (float)M_PI/8.0f, 0.0f);
	dynamicsWorld->addConstraint( rHipConstraint );
	
	//urleg and lrleg
	btHingeConstraint * rKneeConstraint = new btHingeConstraint( *bodies[4], *bodies[5], btVector3(0,-1.25,0), btVector3(0,1.25,0), btVector3(1,0,0), btVector3(1,0,0));
	rKneeConstraint->setLimit(0.0f, (float)M_PI*0.9f);
	dynamicsWorld->addConstraint( rKneeConstraint );

	//lrleg and rfoot
	localA.setOrigin(btVector3(0.0f,-1.0f,0.0f));
	localB.setOrigin(btVector3(0.0f,0.5f,0.5f));
	btConeTwistConstraint * rAnkleConstraint = new btConeTwistConstraint( *bodies[5], *bodies[6], localA, localB );
	rAnkleConstraint->setLimit((float)M_PI/4.0f, (float)M_PI/4.0f, 0.0f);
	dynamicsWorld->addConstraint( rAnkleConstraint );

	//torso and urarm
	localA.setOrigin(btVector3(-0.5f,1.0f,0.0f));
	localB.setOrigin(btVector3(1.5f,0.0f,0.0f));
	btConeTwistConstraint * lShoulderConstraint = new btConeTwistConstraint( *bodies[0], *bodies[7], localA, localB );
	lShoulderConstraint->setLimit((float)M_PI/2.0f, (float)M_PI/2.0f, 0.0f);
	dynamicsWorld->addConstraint( lShoulderConstraint );

	//urarm and lrarm
	btHingeConstraint * rElbowConstraint = new btHingeConstraint( *bodies[10], *bodies[11], btVector3(1,0,0), btVector3(-1.5,0,0), btVector3(0,1,0), btVector3(0,1,0));
	rElbowConstraint->setLimit(-(float)M_PI*0.9f, 0.0f);
	dynamicsWorld->addConstraint( rElbowConstraint );

	//lrarm and rhand
	localA.setOrigin(btVector3(1.0f,0.0f,0.0f));
	localB.setOrigin(btVector3(-0.75f,0.0f,0.0f));
	btConeTwistConstraint * rWristConstraint = new btConeTwistConstraint( *bodies[11], *bodies[12], localA, localB );
	rWristConstraint->setLimit((float)M_PI/2.0f, (float)M_PI/2.0f, 0.0f);
	dynamicsWorld->addConstraint( rWristConstraint );

	//torso and ularm
	localA.setOrigin(btVector3(0.5f,1.0f,0.0f));
	localB.setOrigin(btVector3(-1.5f,0.0f,0.0f));
	btConeTwistConstraint * rShoulderConstraint = new btConeTwistConstraint( *bodies[0], *bodies[10], localA, localB );
	rShoulderConstraint->setLimit((float)M_PI/2.0f, (float)M_PI/2.0f, 0.0f);
	dynamicsWorld->addConstraint( rShoulderConstraint );

	//ularm and llarm
	btHingeConstraint * lElbowConstraint = new btHingeConstraint( *bodies[7], *bodies[8], btVector3(-1,0,0), btVector3(1.5,0,0), btVector3(0,1,0), btVector3(0,1,0));
	lElbowConstraint->setLimit(0.0f, (float)M_PI*0.9f);
	dynamicsWorld->addConstraint( lElbowConstraint );

	//llarm and lhand
	localA.setOrigin(btVector3(-1.0f,0.0f,0.0f));
	localB.setOrigin(btVector3(0.75f,0.0f,0.0f));
	btConeTwistConstraint * lWristConstraint = new btConeTwistConstraint( *bodies[8], *bodies[9], localA, localB );
	lWristConstraint->setLimit((float)M_PI/2.0f, (float)M_PI/2.0f, 0.0f);
	dynamicsWorld->addConstraint( lWristConstraint );


	for (int i = 0; i < 14; i++)
	{
		dynamicsWorld->addRigidBody(bodies[i]);
		bodies[i]->setFriction(0.5f);
	}
	return PhysicsObject::initBullet();
}

int RagdollObject::update(long elapsedTime)
{
	if (!physInit) initBullet();
	return 0;
}

int RagdollObject::render(long totalElapsedTime)
{
	for (int i = 0; i < 14; i++)
	{
		glm::mat4 scale = glm::scale(glm::mat4(), scalings[i]);
		bodies[i]->getWorldTransform().getOpenGLMatrix(glm::value_ptr(worldMatrix));// * scale
		worldMatrix = worldMatrix * scale;
		normalMatrix = glm::transpose(glm::inverse(worldMatrix));
		GameObject::render(totalElapsedTime);
	}
	return 0;
}

