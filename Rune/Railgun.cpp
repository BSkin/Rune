#include "Railgun.h"

Railgun::Railgun(int ownerID, void * owner) : Spell("fireballlauncher.obj", "ctest.tga", "default.glsl", ownerID, owner)
{
	spellID = SPELLID_RAILGUN;
}

Railgun::~Railgun()
{

}

void Railgun::primaryClick()
{
	int damage = 60;
	
	if (owner == NULL) return;
	
	int networkState = getNetworkState();

	btVector3 start = Utilities::glmToBT3(position);
	btVector3 end = start + Utilities::glmToBT3(direction*MAX_DISTANCE);
	CharacterRayTest RayCallback(start, end, owner);
	dynamicsWorld->rayTest(start, end, RayCallback);

	if (RayCallback.hasHit()) {
		end = RayCallback.m_hitPointWorld;
		const btCollisionObject * hitObject = RayCallback.m_collisionObject;

		bool traceError = false;
		if (hitObject != NULL) {
			RigidObject * ownerPointer = ((RigidObject*)((btRigidBody*)hitObject)->getOwner());

			if (ownerPointer != NULL) {
				int charID = ownerPointer->getCharID();

				btVector3 force = 10.0f*safeNormalize(end - start);

				if (networkState == NETWORK_STATE_CLIENT) {
					if (charID > -1) addTCPEvent(createDamageBuffer(ownerID, charID, damage, SPELLID_RAILGUN));
					addTCPEvent(createForceBuffer(ownerPointer, end.x(), end.y(), end.z(), force.x(), force.y(), force.z(), SPELLID_RAILGUN));
				}
				else if (networkState == NETWORK_STATE_SERVER) {
					if (charID > -1) {
						ownerPointer->damage(damage, SPELLID_RAILGUN, ownerID);
						addTCPEvent(createDamageBuffer(ownerID, charID, damage, SPELLID_RAILGUN));
					}
					ownerPointer->applyImpulse(end.x(), end.y(), end.z(), force.x(), force.y(), force.z());
					addTCPEvent(createForceBuffer(ownerPointer, end.x(), end.y(), end.z(), force.x(), force.y(), force.z(), SPELLID_RAILGUN));
				}
				else if (networkState == NETWORK_STATE_OFFLINE) {
					if (charID > -1) ownerPointer->damage(damage, SPELLID_RAILGUN, ownerID);
					ownerPointer->applyImpulse(end.x(), end.y(), end.z(), force.x(), force.y(), force.z());
				}
				else {
					//Error with networkState variable
				}
			}
		}
	}

	if (networkState != NETWORK_STATE_SERVER) {
		glm::vec3 startPoint = Utilities::btToGLM3(&start) + glm::vec3(0, -0.5f, 0);
		glm::vec3 endPoint = Utilities::btToGLM3(&end);
		if (networkState == NETWORK_STATE_CLIENT)
			addTCPEvent(createCreateFadingBeamEffectBuffer(startPoint, endPoint, glm::vec3(1, 0, 0), BEAM_DURATION));
		else if (networkState == NETWORK_STATE_OFFLINE)
			new FadingBeamEffect(startPoint, endPoint, glm::vec3(1, 0, 0), BEAM_DURATION);
	}
}