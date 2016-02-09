#include "Boat.h"

Ocean * Boat::ocean = NULL;
list<GameObject *> * Boat::oceanHeightModObjects = NULL;

Boat::Boat(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path) : PropObject(m, t, s, position, direction, up, path)
{
	oceanHeightModObjects->push_back(this);

	turnRate = 0.0f;

	setMass(10000.0f);
	setFriction(2.0f);
	body->setDamping(0.9f, 0.9f);
	//setGravity(0,-19.62f*2,0);
	//body->setAngularFactor(1.0f);
	collisionType = "vehicle";
		
	buoyancyPoints.push_back(BuoyancyPoint(0.0f, -1.0f, 26.85f));
	buoyancyPoints.push_back(BuoyancyPoint(0.0f, -1.0f, -26.85f));

	buoyancyPoints.push_back(BuoyancyPoint(9.0f, -1.0f, 0.0f));
	buoyancyPoints.push_back(BuoyancyPoint(-9.0f, -1.0f, 0.0f));

	buoyancyPoints.push_back(BuoyancyPoint(9.0f, -1.0f, 14.0f));
	buoyancyPoints.push_back(BuoyancyPoint(-9.0f, -1.0f, 14.0f));

	buoyancyPoints.push_back(BuoyancyPoint(9.0f, -1.0f, -14.0f));
	buoyancyPoints.push_back(BuoyancyPoint(-9.0f, -1.0f, -14.0f));
	
	for (list<BuoyancyPoint>::iterator it = buoyancyPoints.begin(); it != buoyancyPoints.end(); it++)
		(it)->update(&body->getWorldTransform());
}

Boat::~Boat()
{
	oceanHeightModObjects->remove(this);
}

int Boat::update(long elapsedTime)
{
	body->activate(true);

	if (ocean != NULL) {
		for (list<BuoyancyPoint>::iterator it = buoyancyPoints.begin(); it != buoyancyPoints.end(); it++) {
			btVector3 pos = (it)->update(&body->getWorldTransform());
	
			float waterLevel = ocean->getHeight(pos.x(), pos.z());
			if (pos.y() < waterLevel) {
				float buoyantForce = elapsedTime*196.2f/buoyancyPoints.size()*(-pos.y() + waterLevel);
				body->applyImpulse(btVector3(0, buoyantForce, 0), (it)->getLocalPosition());
			}
		}
	}

	//if (turnRate > 0) turnRate -= elapsedTime*0.001f*1.0f;
	//else if (turnRate < 0) turnRate += elapsedTime*0.001f*1.0f;
	//body->setAngularVelocity(btVector3(0,turnRate,0));
	
	return RigidObject::update(elapsedTime);
}


int Boat::render(long totalElapsedTime) 
{
	return RigidObject::render(totalElapsedTime);
}

void Boat::applyImpulseForward(float x, long elapsedTime)
{
	btVector3 forwardVector = body->getWorldTransform() * btVector3(0,0,-1);
	forwardVector.setY(body->getCenterOfMassPosition().y()); //forwardVector = forwardVector.normalized();
	body->applyCentralImpulse(x*elapsedTime*(forwardVector-body->getCenterOfMassPosition()));
}

void Boat::turn(float x, long elapsedTime)
{
	//turnRate += x*elapsedTime*0.001f;
	//body->applyTorque(btVector3(0,elapsedTime*10000,0));
	body->activate(true);
	body->applyTorqueImpulse(btVector3(0, x*elapsedTime, 0));
}