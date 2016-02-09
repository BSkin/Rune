#include "BuoyancyPoint.h"

BuoyancyPoint::BuoyancyPoint(float x, float y, float z)
{
	init(x, y, z);
}

BuoyancyPoint::BuoyancyPoint(glm::vec3 pos)
{
	init(pos.x, pos.y, pos.z);
}

void BuoyancyPoint::init(float x, float y, float z)
{
	localTransformation.setIdentity();
	parentTransform.setIdentity();
	localPosition = btVector3(x, y, z);
	localTransformation.setOrigin(localPosition);
}

btVector3 BuoyancyPoint::update(const btTransform * parentTransform) //returns the x,y,z coordinates in world space
{
	this->parentTransform = *parentTransform;
	//globalTransformation = *parentTransform * localTransformation;
	//btVector3 globalPoint = btVector3(1,1,1);
	return (*parentTransform * localTransformation).getOrigin();
}

btVector3 BuoyancyPoint::getLocalPosition() //returns the x,y,z coordinates relative to the boat
{
	btTransform parentRotation = btTransform(parentTransform.getRotation());
	return parentRotation * localPosition; 
}