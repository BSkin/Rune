#ifndef NETWORK_FUNCTIONS_H
#define NETWORK_FUNCTIONS_H

#include "EventDeclarations.h"
#include "DataBuffer.h"
#include "Packet.h"
#include "LinkedHashMap.h"

#include "GameObject.h"
#include "CharacterObject.h"
#include "PlayerObject.h"
#include "PropObject.h"
#include "MeshObject.h"
#include "FullHealthKit.h"
#include "HalfHealthKit.h"
#include "Fireball.h"
#include "FireballProjectile.h"
#include "GrenadeLauncher.h"
#include "Grenade.h"
#include <algorithm>

typedef int(*FunctionPointer)(DataBuffer*);

class NetworkFunctions
{
public:
	static int translateReceivedEvents(Packet * packet);

	static int executeIndexedFunction(DataBuffer * data);
	static int setConnectedtoHost(bool x) { NetworkFunctions::connectedToHost = x; }
	static void setClientID(int clientID) { NetworkFunctions::clientID = clientID; }
	static void setNetworkState(int x) { NetworkFunctions::networkState = x; }
	static void setStatics(LinkedHashMap<unsigned long, GameObject*> * objects, vector<CharacterObject*> * players) { NetworkFunctions::gameObjects = objects; NetworkFunctions::players = players; }
	static void setEventPackets(Packet * tcp, Packet * udp) { eventPacketTCP = tcp; eventPacketUDP = udp; }

	static int clearObjects(DataBuffer * data);
	static int addPropObject(DataBuffer * data);
	static int addMeshObject(DataBuffer * data);
	static int spawnPlayer(DataBuffer * data);
	static int killGameObject(DataBuffer * data);
	static int deleteGameObject(DataBuffer * data); //Clients are not allowed to call this function
	static int updateGameObject(DataBuffer * data);
	static int addSpell(DataBuffer * data);
	static int changeSpell(DataBuffer * data);
	static int attack(DataBuffer * data);
	static int damage(DataBuffer * data);
	static int heal(DataBuffer * data);
	static int force(DataBuffer * data);
	static int centralForce(DataBuffer * data);
	static int spawnFullHealthKit(DataBuffer * data);
	static int spawnHalfHealthKit(DataBuffer * data);
	static int createFireball(DataBuffer * data);
	static int createGrenade(DataBuffer * data);
	static int createFadingBeamEffect(DataBuffer * data);

	static DataBuffer * createClearObjectsBuffer();
	static DataBuffer * createKillObjectBuffer(unsigned long netID);
	static DataBuffer * createCreateObjectBuffer(GameObject *);
	static DataBuffer * createCreateMeshObjectBuffer(MeshObject *);
	static DataBuffer * createCreatePropObjectBuffer(PropObject *);
	static DataBuffer * createCreateCharacterObjectBuffer(CharacterObject *);
	/*static DataBuffer * createCreateMeshObjectBuffer(int netID);
	static DataBuffer * createCreatePropObjectBuffer(int netID);
	static DataBuffer * createSpawnPlayerObjectBuffer(int netID);*/
	static DataBuffer * createSpawnPlayerRequestBuffer(int playerID, glm::vec3 pos, glm::vec3 lookAt, int classID);
	static DataBuffer * createDeleteObjectBuffer(unsigned long netID);
	static DataBuffer * createUpdateObjectBuffer(unsigned long netID);
	static DataBuffer * createAddSpellBuffer(int playerID, int elementIndex, int spellID);
	static DataBuffer * createChangeSpellBuffer(int playerID, int elementIndex, int subID = 0);
	static DataBuffer * createAttackBuffer(int playerID, int spellID, glm::vec3 pos, glm::vec3 lookAt);
	static DataBuffer * createDamageBuffer(int playerID, int targetID, int damage, int spellID);
	static DataBuffer * createHealBuffer(int playerID, int targetID, int healing, int spellID);	
	static DataBuffer * createForceBuffer(int playerID, unsigned long netID, glm::vec3 worldPos, glm::vec3 force, int spellID);
	static DataBuffer * createForceBuffer(int playerID, unsigned long netID, float worldX, float worldY, float worldZ, float forceX, float forceY, float forceZ, int spellID) { return createForceBuffer(playerID, netID, glm::vec3(worldX, worldY, worldZ), glm::vec3(forceX, forceY, forceZ), spellID); }
	static DataBuffer * createCentralForceBuffer(int playerID, unsigned long netID, glm::vec3 force, int spellID);
	static DataBuffer * createCentralForceBuffer(int playerID, unsigned long netID, float forceX, float forceY, float forceZ, int spellID) { return createCentralForceBuffer(playerID, netID, glm::vec3(forceX, forceY, forceZ), spellID); }
	static DataBuffer * createSpawnFullHealthKitBuffer(glm::vec3 position, unsigned long netID);
	static DataBuffer * createCreateFullHealthKitObjectBuffer(FullHealthKit *);
	static DataBuffer * createSpawnHalfHealthKitBuffer(glm::vec3 position, unsigned long netID);
	static DataBuffer * createCreateHalfHealthKitObjectBuffer(HalfHealthKit *);
	static DataBuffer * createCreateFireballBuffer(int playerID, glm::vec3 pos, glm::vec3 dir);
	static DataBuffer * createCreateGrenadeBuffer(int playerID, glm::vec3 pos, glm::vec3 dir);
	static DataBuffer * createCreateFadingBeamEffectBuffer(glm::vec3 start, glm::vec3 end, glm::vec3 color, long duration);
private:
	static bool connectedToHost;
	static FunctionPointer functionArray[];
	static string functionNames[];
	static int bufferSizes[];

	static int clientID;
	static LinkedHashMap<unsigned long, GameObject*> * gameObjects;
	//static vector<GameObject*> * gameObjects;
	static vector<CharacterObject*> * players;
	static Packet * eventPacketTCP;
	static Packet * eventPacketUDP;

	static int networkState;
	//static bool isServer;
};

#endif