#ifndef RENDER_OBJECT_EFFECT_H
#define RENDER_OBJECT_EFFECT_H

#include "Effect.h"
#include "RenderObject.h"

class RenderObjectEffect : public Effect
{
public:
	RenderObjectEffect(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up);
	virtual ~RenderObjectEffect();

	virtual int update(long elapsedTime);
	virtual int render(long totalElapsedTime);

protected:
	RenderObject * renderObject;
};

#endif