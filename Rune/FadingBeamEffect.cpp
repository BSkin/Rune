#include "FadingBeamEffect.h"

FadingBeamEffect::FadingBeamEffect(glm::vec3 start, glm::vec3 end, glm::vec3 colour, long duration) : BeamEffect(start, end, colour, duration) {}

int FadingBeamEffect::render(long totalElapsedTime)
{
	long time = GetCurrentTime() - startTime;
	if (time < 0) time = 0;
	if (time > duration) time = duration;
	
	float alpha = 1.0f - (float)time / (float)duration;
	renderObject->setShaderParameter("beamColour", glm::vec4(colour.r, colour.g, colour.b, alpha));
	return RenderObjectEffect::render(totalElapsedTime);
}