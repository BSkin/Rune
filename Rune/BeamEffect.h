#ifndef BEAM_EFFECT_H
#define BEAM_EFFECT_H

#include "RenderObjectEffect.h"

class BeamEffect : public RenderObjectEffect
{
public:
	BeamEffect(glm::vec3 start, glm::vec3 end, glm::vec3 colour, long duration);
	virtual ~BeamEffect();

	virtual int update(long elapsedTime);
	virtual int render(long totalElapsedTime);
protected:
	long startTime, duration;
	glm::vec3 colour;
};

#endif