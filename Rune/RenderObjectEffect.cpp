#include "RenderObjectEffect.h"

RenderObjectEffect::RenderObjectEffect(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up) : Effect()
{
	renderObject = new RenderObject(m, t, s, position, direction, up);
}
RenderObjectEffect::~RenderObjectEffect()
{
	delete renderObject;
}

int RenderObjectEffect::update(long elapsedTime)
{
	return renderObject->update(elapsedTime);
}

int RenderObjectEffect::render(long totalElapsedTime)
{
	return renderObject->render(totalElapsedTime);
}