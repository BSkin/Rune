#include "BeamEffect.h"

BeamEffect::BeamEffect(glm::vec3 start, glm::vec3 end, glm::vec3 colour, long duration) : RenderObjectEffect("Beam.obj", "error.tga", "Beam.glsl", start, safeNormalize(end - start), glm::vec3(0, 1, 0))
{
	float beamLength = glm::length(end - start);
	renderObject->setScale(0.1f, 0.1f, beamLength);
	startTime = GetCurrentTime();
	this->colour = colour;
	this->duration = duration;
}

BeamEffect::~BeamEffect() {}

int BeamEffect::update(long elapsedTime)
{
	if (GetCurrentTime() > (startTime + duration)) alive = false;
	return RenderObjectEffect::update(elapsedTime);
}

int BeamEffect::render(long totalElapsedTime)
{
	renderObject->setShaderParameter("beamColour", glm::vec4(colour.r, colour.g, colour.b, 1.0f));
	return RenderObjectEffect::render(totalElapsedTime);
}