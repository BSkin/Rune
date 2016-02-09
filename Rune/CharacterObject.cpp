#include "CharacterObject.h"

float CharacterObject::moveSpeed = 7.0f;  // Ground move speed
float CharacterObject::runAcceleration = 14.0f;   // Ground accel
float CharacterObject::runDeacceleration = 10.0f;   // Deacceleration that occurs when running on the ground
float CharacterObject::airAcceleration = 2.0f;  // Air accel
float CharacterObject::airDeacceleration = 2.0f;  // Deacceleration experienced when opposite strafing
float CharacterObject::airControl = 0.3f;  // How precise air control is
float CharacterObject::sideStrafeAcceleration = 50.0f;   // How fast acceleration occurs to get up to sideStrafeSpeed when side strafing
float CharacterObject::sideStrafeSpeed = 1.0f;    // What the max speed to generate when side strafing
float CharacterObject::jumpSpeed = 8.0f;  // The speed at which the character's up axis gains when hitting jump
float CharacterObject::moveScale = 1.0f;

CharacterObject::CharacterObject(string m, string t, string s, glm::vec3 position, glm::vec3 direction, glm::vec3 up, string path, int playerID, unsigned long netID) : PropObject(m, t, s, position, direction, up, path, netID)
{
	this->playerID = playerID;

	health = maxHealth = 100;

	for (int i = 0; i < 5; i++)
		spells[i] = vector<Spell*>();
	activeSpell = NULL;

	/*if (getNetworkState() == NETWORK_STATE_OFFLINE) {
		addSpell(new Fireball(playerID, this), 1);
		addSpell(new Railgun(playerID, this), 2);
		changeSpell(1);
	}*/

	addSpell(new Shotgun(playerID, this), 0);
	addSpell(new Fireball(playerID, this), 1);
	addSpell(new GrenadeLauncher(playerID, this), 2);
	addSpell(new Railgun(playerID, this), 3);
	if (getNetworkState() == NETWORK_STATE_OFFLINE) changeSpell(1);

	airborne = false;
	body->setFriction(0.5f);
	setMass(5.0f);
	body->setAngularFactor(0);
	//setGravity(0.0f, -9.81f*4.0f, 0.0f);
	this->direction = glm::vec3(0,0,1);
	this->left = glm::vec3(1,0,0);
	this->up = glm::vec3(0,1,0);
	floorDist = 1.0f;
	
	groundNormal = btVector3(0, 1, 0);
	frameTurnAmount = glm::vec2(0, 0);
	directionAngle = (float)(180.0 / M_PI) * glm::vec2(-atan2(direction.x, direction.z), asin(-direction.y));
	localWalkDirection = glm::vec2(0, 0);

	airborneTest = createAirborneTest();
	vehicle = NULL;
	vehicleSpeed = btVector3(0,0,0);
}
CharacterObject::~CharacterObject()
{
	delete airborneTest;
	for (int i = 0; i < 5; i++) {
		for (int i2 = 0; i2 < (int)spells[i].size(); i2++)
			delete spells[i][i2];
	}
}

btRigidBody * CharacterObject::createAirborneTest()
{
	btRigidBody * temp;
	btCollisionShape * colShape = PhysicsObject::getCollisionShape("airborneSphere");
	colShape->setMargin(1.0f);

	/// Create Dynamic Objects
	btTransform startTransform;
	startTransform.setIdentity();

	btScalar mass = 1.0f;
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass,localInertia);
		
	startTransform.setOrigin(btVector3(body->getWorldTransform().getOrigin()));

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	temp = new btRigidBody(mass,myMotionState,colShape,localInertia, this);
	delete myMotionState;
	return temp;
}

int CharacterObject::update(long elapsedTime)
{	
	body->activate(true);
	double speed = 20.0f;
	double e = elapsedTime*0.001;
	
	if (noVehicleTimer <= 0l) vehicle = NULL;
	else noVehicleTimer -= elapsedTime;
	airborne = true;
	testGround();

	
	//setGravity(0,-9.81f*2.0f,0);
	if (airborne) {
		if (magSqr(walkDirection) != 0.0 && walkDirection != glm::vec3(0,0,0))
			airwalk(glm::normalize(walkDirection), elapsedTime);
	}
	else {
		if (walkDirection != glm::vec3(0,0,0))
			walk(glm::normalize(walkDirection) * (float)speed, elapsedTime);
		if (walkDirection == glm::vec3(0,0,0)) {
			if ((vehicle == 0 && magSqr(&body->getLinearVelocity()) < 400.0f) ||
				(vehicle != 0 && magSqr(&(body->getLinearVelocity() - vehicle->getVelocity())) < 400.0f)) {
				stopWalking(elapsedTime);
				//setGravity(0,0,0);
			}
		}
	}
	
	walkDirection = glm::vec3(0,0,0);
	localWalkDirection = glm::vec2(0, 0);
		
	if (vehicle != NULL) {
		turn((float)(-vehicle->getAngularVelocity().y()*elapsedTime*0.180/M_PI), 0.0f);
		btVector3 localPos = (getBTPosition() - vehicle->getBTPosition()).rotate(btVector3(0,1,0), (float)(vehicle->getAngularVelocity().y()*elapsedTime*0.00095));
		body->translate(localPos - (getBTPosition() - vehicle->getBTPosition()));
	}
	
	//for (list<Spell*>::iterator iter = weaponList.begin(); iter != weaponList.end(); iter++) 
	//	(*iter)->updateSpell(btToGLM3(&getPosition()), direction, up, elapsedTime);
	if (activeSpell != NULL)
		activeSpell->updateSpell(btToGLM3(&getBTPosition()) + glm::vec3(0,1,0), getDirection(), up, elapsedTime);

	btTransform trasnform;
	trasnform.setIdentity();
	trasnform.setOrigin(body->getCenterOfMassPosition());
	body->setCenterOfMassTransform(trasnform);

	return RigidObject::update(elapsedTime);
}

int CharacterObject::render(long totalElapsedTime)
{
	//for (list<Spell*>::iterator iter = weaponList.begin(); iter != weaponList.end(); iter++) 
	//	(*iter)->render(totalElapsedTime);
	if (activeSpell != NULL)
		activeSpell->render(totalElapsedTime);
	return RigidObject::render(totalElapsedTime);
}

void CharacterObject::walkForward()
{	
	walkDirection += direction;
	localWalkDirection.y += 1.0f;
}
void CharacterObject::walkBackward()
{ 
	walkDirection -= direction;
	localWalkDirection.y -= 1.0f;
}
void CharacterObject::walkLeft()
{ 
	walkDirection += left; 
	localWalkDirection.x += 1.0f;
}
void CharacterObject::walkRight()
{ 
	walkDirection -= left; 
	localWalkDirection.x -= 1.0f;
}

void CharacterObject::turn(float x, float y)
{
	frameTurnAmount = glm::vec2(x, y);
	/*direction = glm::rotate(direction, y, left);
	direction = glm::rotate(direction, x, -up);
	left = glm::normalize(glm::cross(up, direction));
	*/

	if (directionAngle.y + y < -89.0f)		directionAngle.y = -89.0f - y;
	else if (directionAngle.y + y > 89.0f)	directionAngle.y = 89.0f - y;
	directionAngle.y += y;

	directionAngle.x += x;
	if (directionAngle.x > 180.0f) directionAngle.x -= 360.0f;
	if (directionAngle.x < -180.0f) directionAngle.x += 360.0f;

	//direction = glm::rotate(direction, y, left);
	//direction = glm::rotate(direction, x, -up);

	direction = glm::rotate(glm::vec3(0, 0, 1), directionAngle.x, -up);
	left = glm::normalize(glm::cross(up, direction));
	direction = glm::rotate(direction, directionAngle.y, left);
}

void CharacterObject::turnLeft(float angle) { turn(-angle, 0.0f); }
void CharacterObject::turnRight(float angle) { turn(angle, 0.0f); }
void CharacterObject::turnUp(float angle) { turn(0.0f, -angle); }
void CharacterObject::turnDown(float angle) { turn(0.0f, angle); }

float changeSpeed(float input, float target, float dif) {
	if (target == input) return target;
	if (abs(target-input) < dif) return target;
	float x = (target-input)/abs(target-input)*dif;
	return input + x;
}

btVector3 changeVelocity(btVector3 * currentVel, btVector3 * targetVel, float dif) {
	btVector3 newVel;

	float currentSpeed = currentVel->length();
	float xFactor = abs((targetVel->x() - currentVel->x()) / currentSpeed);
	float yFactor = abs((targetVel->y() - currentVel->y()) / currentSpeed);
	float zFactor = abs((targetVel->z() - currentVel->z()) / currentSpeed);

	newVel.setX(changeSpeed(currentVel->x(), targetVel->x(), dif * xFactor));
	newVel.setY(changeSpeed(currentVel->y(), targetVel->y(), dif *yFactor));
	newVel.setZ(changeSpeed(currentVel->z(), targetVel->z(), dif *zFactor));

	return newVel;
}

void CharacterObject::walk(float x, float y, float z, long elapsedTime)
{
	if (magnitude(&body->getLinearVelocity()) <= maxSpeed)
	{
		btVector3 sideV = (btVector3(x,y,z).cross(-groundNormal)).normalized();
		btVector3 forwardV = (sideV.cross(-groundNormal)).normalized();

		btVector3 v = body->getLinearVelocity();
		btVector3 forward = -forwardV * magnitude(&btVector3(x,y,z));

		if (vehicle == 0)
			body->setLinearVelocity(changeVelocity(&v, &forward, 0.05f*elapsedTime));
		else {
			body->setLinearVelocity(changeVelocity(&v, &(forward + vehicle->getVelocity()), 0.05f*elapsedTime));
			//body->setLinearVelocity(forward + vehicle->getVelocity());
		}
	}
}

void CharacterObject::stopWalking(long elapsedTime) {
	btVector3 v = body->getLinearVelocity();
	if (vehicle == 0)
		body->setLinearVelocity(changeVelocity(&v, &btVector3(0,0,0), 0.05f*elapsedTime));
	else
		body->setLinearVelocity(changeVelocity(&v, &vehicle->getVelocity(), 0.05f*elapsedTime));
	//	body->setLinearVelocity(vehicle->getVelocity());
}

void CharacterObject::airwalk(float x, float y, float z, long elapsedTime)
{
	//body->applyCentralImpulse(btVector3(x,y,z));
	//return;
	
	glm::vec3 wishdir;
	float wishvel = airAcceleration;
	float accel;

	//var scale = CmdScale();
	
	wishdir = glm::vec3(localWalkDirection.x, 0, localWalkDirection.y);
	wishdir = direction * localWalkDirection.y + left * localWalkDirection.x; // ???

	//float wishspeed = body->getLinearVelocity().length();// 20.0f; // wishdir.length();
	float wishspeed = 20.0f; // wishdir.length();

	wishdir = glm::normalize(wishdir);
	glm::vec3 wishDirNorm = wishdir;
	//wishspeed *= scale;

	// CPM: Aircontrol
	float wishspeed2 = wishspeed;
	glm::vec3 playerVelocity = btToGLM3(&getVelocity());
	if (glm::dot(playerVelocity, wishdir) < 0.0f)
		accel = airDeacceleration;
	else
		accel = airAcceleration;
	// If the player is ONLY strafing left or right
	if (localWalkDirection.y == 0.0f && localWalkDirection.x != 0.0f)
	{
		if (wishspeed > sideStrafeSpeed)
			wishspeed = sideStrafeSpeed;
		accel = sideStrafeAcceleration;
	}

	Accelerate(wishdir, wishspeed, accel, elapsedTime);
	
	if (airControl) AirControl(wishdir, wishspeed2, elapsedTime);
	// !CPM: Aircontrol

	// LEGACY MOVEMENT SEE BOTTOM
}

/**
* Calculates wish acceleration based on player's cmd wishes
*/
void CharacterObject::Accelerate(glm::vec3 wishdir, float wishspeed, float accel, long elapsedTime)
{
	float addspeed;
	float accelspeed;
	float currentspeed;

	glm::vec3 playerVelocity = btToGLM3(&body->getLinearVelocity());
	currentspeed = glm::dot(playerVelocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
		return;
	accelspeed = accel * wishspeed * elapsedTime * 0.001f;
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	playerVelocity.x += accelspeed * wishdir.x;
	playerVelocity.z += accelspeed * wishdir.z;

	setVelocity(playerVelocity.x, playerVelocity.y, playerVelocity.z);
}

/**
* Air control occurs when the player is in the air, it allows
* players to move side to side much faster rather than being
* 'sluggish' when it comes to cornering.
*/
void CharacterObject::AirControl(glm::vec3 wishdir, float wishspeed, long elapsedTime)
{
	float zspeed;
	float speed;
	float dot;
	float k;

	// Can't control movement if not moving forward or backward
	//if (cmd.forwardmove == 0 || wishspeed == 0)
	//if (localWalkDirection.y == 0.0f || wishspeed == 0.0f)
	glm::vec3 playerVelocity = btToGLM3(&body->getLinearVelocity());
	if ((playerVelocity.x == 0.0f && playerVelocity.z == 0.0f) || wishspeed == 0.0f)
		return;

	zspeed = playerVelocity.y;
	playerVelocity.y = 0.0f;
	// Next two lines are equivalent to idTech's VectorNormalize() /
	speed = glm::length(playerVelocity);// sqrt(playerVelocity.x*playerVelocity.x + playerVelocity.z*playerVelocity.z);
	playerVelocity = glm::normalize(playerVelocity);

	dot = glm::dot(playerVelocity, wishdir);
	k = 32.0f;
	k *= airControl * dot * dot * elapsedTime * 0.001f;

	// Change direction while slowing down
	if (dot > 0.0f)
	{
		playerVelocity.x = playerVelocity.x * speed + wishdir.x * k;
		playerVelocity.y = playerVelocity.y * speed + wishdir.y * k;
		playerVelocity.z = playerVelocity.z * speed + wishdir.z * k;

		playerVelocity = glm::normalize(playerVelocity);
		//moveDirectionNorm = playerVelocity;
	}

	playerVelocity.x *= speed;
	playerVelocity.y = zspeed; // Note this line
	playerVelocity.z *= speed;
	
	body->setLinearVelocity(glmToBT3(playerVelocity));
}

void CharacterObject::jump()
{
	if (airborne) return;
	body->setLinearVelocity(body->getLinearVelocity() + btVector3(0,17.0f,0));
	body->translate(btVector3(0,0.2f,0));
}

void CharacterObject::testGround()
{
	string foo = "asdf";
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(body->getCenterOfMassPosition());
	airborneTest->setCenterOfMassTransform(transform);
	AirborneContactCallback callback(*airborneTest, foo, &airborne, &groundNormal, &vehicle);
	dynamicsWorld->updateAabbs();
	dynamicsWorld->contactTest(airborneTest,callback);

	if (vehicle != NULL)
		noVehicleTimer = 500; //in ms
	//groundNormal = btVector3(0,1,0);
}

void CharacterObject::setDirection(glm::vec3 direction)
{
	this->direction = glm::normalize(direction);
	up = glm::vec3(0,1,0);
	left = glm::normalize(glm::cross(up, direction));
	
	/*btScalar angle = acos(glmToBT3(up).dot(glmToBT3(direction)));
	btVector3 axis = glmToBT3(up).cross(glmToBT3(direction));
	btTransform transform = body->getCenterOfMassTransform();
	transform.setRotation(btQuaternion(axis, angle));
	body->setCenterOfMassTransform(transform);*/
}

void CharacterObject::setDirection(glm::vec3 direction, glm::vec3 left, glm::vec3 up)
{
	this->direction = glm::normalize(direction);
	this->left = glm::normalize(left);
	this->up = glm::normalize(up);
}

void CharacterObject::addSpell(Spell * spell, int elementIndex)
{
	if (spell == NULL || elementIndex < 0 || elementIndex > 4) return;
	spells[elementIndex].push_back(spell);
}

DataBuffer * CharacterObject::createChangeSpellBuffer(int elementIndex, int subID) //SubID refers to the index in the element vector, IE the Xth fire spell
{
	if (elementIndex < 0 || elementIndex > 4) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(17);
	int functionIndex = EVENT_CHANGE_SPELL;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &playerID, 4);
	tempBuffer->copy(8, &elementIndex, 4);
	tempBuffer->copy(12, &subID, 4);
	tempBuffer->copy(16, "\n", 1);
	return tempBuffer;
}

Spell * CharacterObject::getSpell(int elementIndex, int subIndex)
{
	if (elementIndex < 0 || elementIndex > 4) return NULL;
	if (subIndex < 0 || subIndex >= spells[elementIndex].size()) return NULL;

	return spells[elementIndex][subIndex];
}

void CharacterObject::getActiveSpellIndices(int * outElementIndex, int * outSubIndex) {
	if (outElementIndex == NULL || outSubIndex == NULL) return;
	*outElementIndex = currentElementIndex;
	*outSubIndex = currentSpellIndex;
}

int CharacterObject::changeSpell(int elementIndex, int subIndex)
{
	int ret = syncSpell(elementIndex, subIndex);
	if (ret != 0) return ret;

	int networkState = getNetworkState();
	if (networkState == NETWORK_STATE_CLIENT || networkState == NETWORK_STATE_SERVER) addTCPEvent(createChangeSpellBuffer(currentElementIndex, currentSpellIndex));
	return 0;
}

int CharacterObject::syncSpell(int elementIndex, int subIndex)
{
	if (elementIndex < 0 || elementIndex > 4) return -1;
	if (subIndex < 0 || subIndex >= spells[elementIndex].size()) return -1;

	currentElementIndex = elementIndex;
	currentSpellIndex = subIndex;

	activeSpell = spells[currentElementIndex][currentSpellIndex];
	return 0;
}

void CharacterObject::cycleSpell(int elementIndex)
{
	if (elementIndex < 0 || elementIndex > 4) return;
	if (spells[elementIndex].size() == 0) return;
	if (currentElementIndex != elementIndex) {
		currentElementIndex = elementIndex;
		currentSpellIndex = 0;
	}
	else {
		currentSpellIndex++;
		currentSpellIndex = currentSpellIndex % spells[currentElementIndex].size();
	}

	changeSpell(currentElementIndex, currentSpellIndex);
	//activeSpell = spells[currentElementIndex][currentSpellIndex];
}

/*
functionID	4
objectIndex	4
posX		4
posY		4
posZ		4
linVelX		4
linVelY		4
linVelZ		4
orientW		4
orientX		4
orientY		4
orientZ		4
angVelX		4
angVelY		4
angVelZ		4*/

DataBuffer * CharacterObject::serialize()
{
	if (!physInit) return NULL;

	btVector3 pos = body->getCenterOfMassPosition();
	btVector3 linVel = body->getLinearVelocity();
	//btQuaternion orient = body->getOrientation();
	btVector3 angVel = body->getAngularVelocity();

	DataBuffer * tempBuffer = new DataBuffer(getSerializedSize());

	float test = 1.0f;
	memcpy(&test, &pos.x(), 4);

	int uninit = -1;
	tempBuffer->copy(0, &uninit, 4); //function ID

	unsigned long netID = getNetID();
	tempBuffer->copy(4, &netID, 4);
	tempBuffer->copy(8, &pos.x(), 4);
	tempBuffer->copy(12, &pos.y(), 4);
	tempBuffer->copy(16, &pos.z(), 4);
	tempBuffer->copy(20, &linVel.x(), 4);
	tempBuffer->copy(24, &linVel.y(), 4);
	tempBuffer->copy(28, &linVel.z(), 4);
	tempBuffer->copy(32, &directionAngle.x, 4);
	tempBuffer->copy(36, &directionAngle.y, 4);
	tempBuffer->copy(40, &angVel.x(), 4);
	tempBuffer->copy(44, &angVel.y(), 4);
	tempBuffer->copy(48, &angVel.z(), 4);

	tempBuffer->copy(52, "\n", 1);

	return tempBuffer;
}

int CharacterObject::deserialize(DataBuffer * data)
{
	if (!physInit || data == NULL || data->getSize() != getSerializedSize()) return -1;

	btVector3 pos = body->getCenterOfMassPosition();
	btVector3 linVel = body->getLinearVelocity();
	btQuaternion orient = body->getOrientation();
	btVector3 angVel = body->getAngularVelocity();

	float floatData[12];
	memcpy(&floatData[0], &data->getData()[8], 4);
	memcpy(&floatData[1], &data->getData()[12], 4);
	memcpy(&floatData[2], &data->getData()[16], 4);
	memcpy(&floatData[3], &data->getData()[20], 4);
	memcpy(&floatData[4], &data->getData()[24], 4);
	memcpy(&floatData[5], &data->getData()[28], 4);
	memcpy(&floatData[6], &data->getData()[32], 4);
	memcpy(&floatData[7], &data->getData()[36], 4);
	memcpy(&floatData[8], &data->getData()[40], 4);
	memcpy(&floatData[9], &data->getData()[44], 4);
	memcpy(&floatData[10], &data->getData()[48], 4);
	
	directionAngle = glm::vec2(floatData[6], floatData[7]);
	direction = glm::rotate(glm::vec3(0, 0, 1), directionAngle.x, -up);
	left = glm::normalize(glm::cross(up, direction));
	direction = glm::rotate(direction, directionAngle.y, left);

	btScalar angle = acos(glmToBT3(up).dot(glmToBT3(direction)));
	btVector3 axis = glmToBT3(up).cross(glmToBT3(direction));
	//btTransform newTransform = btTransform(btQuaternion(axis, angle), btVector3(floatData[0], floatData[1], floatData[2]));
	//btTransform newTransform = btTransform(btQuaternion(-directionAngle.x / 180.0f*(float)M_PI, 0, 0), btVector3(floatData[0], floatData[1], floatData[2]));
	btTransform newTransform;
	newTransform.setIdentity();
	newTransform.setOrigin(btVector3(floatData[0], floatData[1], floatData[2]));
	body->setCenterOfMassTransform(newTransform);
	//body->setCenterOfMassTransform(newTransform);
	body->setLinearVelocity(btVector3(floatData[3], floatData[4], floatData[5]));
	body->setAngularVelocity(btVector3(floatData[8], floatData[9], floatData[10]));

	return 0;
}

int CharacterObject::setMaxHealth(int x)
{
	if (x < 1) return 0;
	maxHealth = x;
	return 1;
}

int CharacterObject::getMaxHealth(int x) { return maxHealth; }

DataBuffer * CharacterObject::createDamageBuffer(int damage, int spellID, int attackerID)
{
	if (spellID < 0 || spellID >= SPELLID_COUNT) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(21);
	int functionIndex = EVENT_DAMAGE;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &attackerID, 4); //attacker
	tempBuffer->copy(8, &playerID, 4); //target
	tempBuffer->copy(12, &damage, 4);
	tempBuffer->copy(16, &spellID, 4);
	tempBuffer->copy(20, "\n", 1);
	return tempBuffer;
}

int CharacterObject::damage(int x, int spellID, int attackerID) //returns effective damage
{
	if (alive == false) return 0;

	int preHealth = health;
	bool kill = false;

	health -= x;
	if (health < 0) health = 0;
	if (health > maxHealth) health = maxHealth;
	if (health <= 0) {
		alive = false;
		kill = true;
	}

	int effectiveDamage = preHealth - health;
	if (getNetworkState() == NETWORK_STATE_SERVER) addTCPEvent(createDamageBuffer(x, spellID, attackerID));
	//log damage (x, effectiveDamage, spellID, attackerID, myID, kill)
	if (effectiveDamage != x) printf("Player %d dealt %d damage (%d overkill) to Player %d with %s\n", attackerID, effectiveDamage, x - effectiveDamage, playerID, Spell::getSpellName(spellID).c_str());
	else printf("Player %d dealt %d damage to Player %d with %s\n", attackerID, x, playerID, Spell::getSpellName(spellID).c_str());
	
	return effectiveDamage;
}

DataBuffer * CharacterObject::createHealBuffer(int healing, int spellID, int healerID)
{
	if (spellID < 0 || spellID >= SPELLID_COUNT) return NULL;

	DataBuffer * tempBuffer = new DataBuffer(21);
	int functionIndex = EVENT_HEAL;
	tempBuffer->copy(0, &functionIndex, 4);
	tempBuffer->copy(4, &healerID, 4); //attacker
	tempBuffer->copy(8, &playerID, 4); //target
	tempBuffer->copy(12, &healing, 4);
	tempBuffer->copy(16, &spellID, 4);
	tempBuffer->copy(20, "\n", 1);
	return tempBuffer;
}

int CharacterObject::heal(int x, int spellID, int healerID) //returns effective healing
{
	if (alive == false) return 0;

	int preHealth = health;
	bool kill = false;

	health += x;
	if (health < 0) health = 0;
	if (health > maxHealth) health = maxHealth;

	if (health <= 0) {
		kill = true;
		alive = false;
	}

	int effectiveHealing = health - preHealth;
	if (getNetworkState() == NETWORK_STATE_SERVER) addTCPEvent(createHealBuffer(x, spellID, healerID));
	//log heal (x, effectiveHealing, spellID, healerID, myID, kill)
	if (effectiveHealing != x) printf("Player %d healed Player %d for %d health (%d overheal) with %s\n", healerID, playerID, effectiveHealing, x - effectiveHealing, Spell::getSpellName(spellID).c_str());
	else printf("Player %d healed Player %d for %d health with %s\n", healerID, playerID, x, Spell::getSpellName(spellID).c_str());
	
	return effectiveHealing;
}

int CharacterObject::healPercentage(float x, int spellID, int healerID)
{
	return heal((int)(maxHealth * x + 0.5f), spellID, healerID);
}