#ifndef FADING_BEAM_EFFECT_H
#define	FADING_BEAM_EFFECT_H

#include "BeamEffect.h"

class FadingBeamEffect : public BeamEffect
{
public:
	FadingBeamEffect(glm::vec3 start, glm::vec3 end, glm::vec3 colour, long duration);
	virtual int render(long totalElapsedTime);
};

#endif