#include "HeightFieldObject.h"

HeightFieldObject::HeightFieldObject(string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up, float * heightData, int width, int length, float offsetY, int colGroup, int colMask) : RigidObject("", t, s, position, direction, up, "heightField", colGroup, colMask)
{
	this->heightData = 0;
	model = Model::loadHeightFieldModel(heightData, width, length);
	initBullet(heightData, width, length, offsetY, colGroup, colMask);
}

HeightFieldObject::~HeightFieldObject()
{
	delete colShape; colShape = 0;
	delete model; model = 0;
	delete[] heightData;
}

int HeightFieldObject::initBullet(float * heightData, int width, int length, float offsetY, int colGroup, int colMask)
{
	#define MAX_HEIGHT 1000.0f
	#define MIN_HEIGHT -1000.0f
	this->heightData = heightData;
	this->position.y += offsetY;
	colShape = new btHeightfieldTerrainShape(width, length, heightData, 1.0f, MIN_HEIGHT, MAX_HEIGHT, 1, PHY_FLOAT, true);

	return RigidObject::initBullet("heightField", colGroup, colMask);
}