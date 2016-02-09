#ifndef BOAT_H
#define	BOAT_H

#include "PropObject.h"
#include "BuoyancyPoint.h"
#include "Ocean.h"

#include "glm/glm.hpp"						//glm::vec3, glm::vec4, glm::ivec4, glm::mat4
#include "glm/gtc/matrix_transform.hpp"		//glm::translate, glm::rotate, glm::scale
#include "glm/gtc/type_ptr.hpp"				//glm::make_mat4
#include "glm/gtx/rotate_vector.hpp"

#define WATERLEVEL 0

class Boat : public PropObject
{
public:
	Boat();
	Boat(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path); //ok for now
	virtual ~Boat();

	virtual int update(long elapsedTime);
	virtual int render(long totalElapsedTime);

	void applyImpulseForward(float x, long elapsedtime);
	void turn(float x, long elapsedTime);

	static void setStatics(list<GameObject *> * oceanHeightObjects) { oceanHeightModObjects = oceanHeightObjects; }
	static void setOcean(Ocean * o) { ocean = o; }
private:
	float turnRate;

	list<BuoyancyPoint> buoyancyPoints;

	static Ocean * ocean;
	static list<GameObject *> * oceanHeightModObjects;
};

#endif