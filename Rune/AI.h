#ifndef AI_H
#define AI_H

#include "CharacterObject.h"

class AI : public CharacterObject
{
public:
	AI();
	AI(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, int playerID); //ok for now
	virtual ~AI();
	
	virtual int update(long elapsedTime);
	virtual int render(long totalElapsedTime);
};

#endif