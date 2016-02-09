#include "Shotgun.h"

Shotgun::Shotgun(int ownerID, void * owner) : Spell("fireballlauncher.obj", "floor.tga", "default.glsl", ownerID, owner)
{
	spellID = SPELLID_SHOTGUN;
}

Shotgun::~Shotgun()
{

}

struct PlayerDamage {
	int playerID;
	int damage;
	RigidObject * hitPlayer;
};

PelletHitData * Shotgun::getHitData(btVector3 &start, btVector3 &end) //returns playerID or -1 if no hit
{
	CharacterRayTest RayCallback(start, end, owner);
	dynamicsWorld->rayTest(start, end, RayCallback);

	PelletHitData * hitData = new PelletHitData();
	hitData->playerID = -1;
	hitData->hitObject = NULL;
	hitData->end = end;

	if (RayCallback.hasHit()) {
		hitData->end = end = RayCallback.m_hitPointWorld;
		const btCollisionObject * hitObject = RayCallback.m_collisionObject;
		hitData->hitObject = ((RigidObject*)((btRigidBody*)hitObject)->getOwner());

		if (hitObject != NULL) {
			RigidObject * ownerPointer = ((RigidObject*)((btRigidBody*)hitObject)->getOwner());
			if (ownerPointer != NULL) hitData->playerID = ownerPointer->getCharID();
		}
	}

	return hitData;
}

void Shotgun::primaryClick()
{
	int pelletDamage = 10;
	float pelletSpread = 5.0f / 180.0f * M_PI; //radians
	
	if (owner == NULL) return;

	list<PelletHitData*> hitData = list<PelletHitData*>();
	list<PlayerDamage> playerDamageList = list<PlayerDamage>();
	int networkState = getNetworkState();

	btVector3 pelletDirection = Utilities::glmToBT3(direction);
	btVector3 start = Utilities::glmToBT3(position);
	btVector3 end = start + pelletDirection * MAX_DISTANCE;
	
	hitData.push_back(getHitData(start, end));

	pelletDirection = pelletDirection.rotate(Utilities::glmToBT3(up), pelletSpread);
	for (int i = 0; i < 8; i++) {
		end = start + pelletDirection * MAX_DISTANCE;
		hitData.push_back(getHitData(start, end));
		pelletDirection = pelletDirection.rotate(Utilities::glmToBT3(direction), M_PI / 4.0f);
	}

	while (hitData.size() > 0) {
		if (hitData.back()->playerID >= 0) {
			bool inList = false;
			for (list<PlayerDamage>::iterator iter = playerDamageList.begin(); iter != playerDamageList.end(); iter++) {
				if (iter->playerID == hitData.back()->playerID) {
					iter->damage += pelletDamage;
					inList = true;
					break;
				}
			}
			if (!inList) {
				PlayerDamage p; 
				p.playerID = hitData.back()->playerID;
				p.damage = pelletDamage;
				p.hitPlayer = hitData.back()->hitObject;
				playerDamageList.push_back(p);
			}
		}

		if (hitData.back()->hitObject != NULL) {
			end = hitData.back()->end;
			btVector3 force = PELLET_FORCE*safeNormalize(end - start);

			if (networkState == NETWORK_STATE_CLIENT) {
				addTCPEvent(createForceBuffer(hitData.back()->hitObject, end.x(), end.y(), end.z(), force.x(), force.y(), force.z(), SPELLID_SHOTGUN));
			}
			else if (networkState == NETWORK_STATE_SERVER) {
				hitData.back()->hitObject->applyImpulse(end.x(), end.y(), end.z(), force.x(), force.y(), force.z());
				addTCPEvent(createForceBuffer(hitData.back()->hitObject, end.x(), end.y(), end.z(), force.x(), force.y(), force.z(), SPELLID_SHOTGUN));
			}
			else if (networkState == NETWORK_STATE_OFFLINE) {
				hitData.back()->hitObject->applyImpulse(end.x(), end.y(), end.z(), force.x(), force.y(), force.z());
			}
			else {
				//Error with networkState variable
			}
		}

		if (networkState != NETWORK_STATE_SERVER) {
			if (networkState == NETWORK_STATE_CLIENT)
				addTCPEvent(createCreateFadingBeamEffectBuffer(Utilities::btToGLM3(&start) + glm::vec3(0, -0.5, 0), Utilities::btToGLM3(&hitData.back()->end), glm::vec3(0.5f, 0.5f, 0.5f), BEAM_DURATION));
			else if (networkState == NETWORK_STATE_OFFLINE)
				new FadingBeamEffect(Utilities::btToGLM3(&start) + glm::vec3(0, -0.5, 0), Utilities::btToGLM3(&hitData.back()->end), glm::vec3(0.5f, 0.5f, 0.5f), BEAM_DURATION);
		}

		delete hitData.back();
		hitData.pop_back();
	}

	while (playerDamageList.size() > 0) {
		int charID = playerDamageList.back().playerID;
		if (networkState == NETWORK_STATE_CLIENT) {
			if (charID > -1) addTCPEvent(createDamageBuffer(ownerID, charID, playerDamageList.back().damage, SPELLID_SHOTGUN));
		}
		else if (networkState == NETWORK_STATE_SERVER) {
			if (charID > -1) {
				playerDamageList.back().hitPlayer->damage(playerDamageList.back().damage, SPELLID_SHOTGUN, ownerID);
				addTCPEvent(createDamageBuffer(ownerID, charID, playerDamageList.back().damage, SPELLID_SHOTGUN));
			}
		}
		else if (networkState == NETWORK_STATE_OFFLINE) {
			if (charID > -1) playerDamageList.back().hitPlayer->damage(playerDamageList.back().damage, SPELLID_SHOTGUN, ownerID);
		}
		else {
			//Error with networkState variable
		}

		playerDamageList.pop_back();
	}
}