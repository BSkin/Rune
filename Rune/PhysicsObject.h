#ifndef PHYSICSOBJECT_H
#define PHYSICSOBJECT_H

#include "GameObject.h"
#include "Bullet\src\btBulletDynamicsCommon.h"
#include "Bullet\src\BulletCollision\CollisionShapes\btBvhTriangleMeshShape.h"

#include <unordered_map>
#include <map>
using std::unordered_map;
using std::make_pair;
#define my_map unordered_map<string, btCollisionShape *>

#define BIT(x) (1<<(x))
#define COL_NONE	0
#define COL_STRICT	BIT(0)
#define COL_DEFAULT	BIT(0) | BIT(1)
#define COL_PROJ	BIT(1)

struct CollisionShape {
	string type;
	string pos;
	string size;
	string axis;
	string rot;
	list<btVector3> vertices;
	CollisionShape() : type(""), pos(""), size(""), axis(""), rot("")
	{
		vertices = list<btVector3>();
	}
};

class PhysicsObject : public GameObject
{
public:
	PhysicsObject();
	PhysicsObject(string, string, string); //ok for now
	PhysicsObject(string, string, string, glm::vec3 position, glm::vec3 direction, glm::vec3 up, unsigned long netID = 0); //ok for now
	virtual ~PhysicsObject() = 0;

	virtual int render(long elapsedTime);
	virtual int update(long elapsedTime);

	virtual void setVelocity(glm::vec3 vel) {}// { body->setLinearVelocity(btVector3(vel.x, vel.y, vel.z)); }
	virtual void setVelocity(float x, float y, float z) {}// { body->setLinearVelocity(btVector3(x, y, z)); }
	virtual void setGravity(glm::vec3 grav) {}// { body->setGravity(btVector3(grav.x, grav.y, grav.z)); }
	virtual void setGravity(float x, float y, float z) {}// { body->setGravity(btVector3(x, y, z)); }
	virtual void setRotation(glm::vec3 lookAt, glm::vec3 up) {}
	virtual glm::vec3 getPosition() { return GameObject::getPosition(); }
	virtual btVector3 getBTPosition() { return btVector3(0, 0, 0); }
	virtual void wakeUp() {}
	virtual void applyCentralImpulse(float x, float y, float z) {}
	virtual void applyCentralImpulse(glm::vec3 x) {}
	virtual void applyImpulse(float worldX, float worldY, float worldZ, float forceX, float forceY, float forceZ) {}
	virtual void applyImpulse(glm::vec3 worldPos, glm::vec3 force) {}
	virtual void applyImpulse(btVector3 &worldPos, btVector3 &force) {}

	string getCollisionType() { return collisionType; }

	virtual string getTypeName() { return "PhysicsObject"; }

	static void setStatics(btAlignedObjectArray<btCollisionShape*> * col) { collisionShapes = col; }
	//static void setStatics(btAlignedObjectArray<btCollisionShape*> * col, btDiscreteDynamicsWorld * dyn) { collisionShapes = col; dynamicsWorld = dyn; }
	static btCollisionShape * readCollisionData(char * path);
	static void populateCollisionLibrary();
	static void deleteCollisionLibrary();
	static btCollisionShape * getCollisionShape(string key);
protected:
	virtual int initBullet();
	static btCollisionShape * createCollisionShape(CollisionShape * collisionShapeData);
	static btCollisionShape * createConvexHullShape(list<btVector3> * pointList);

	static btAlignedObjectArray<btCollisionShape*> * collisionShapes;
	//static btDiscreteDynamicsWorld * dynamicsWorld;
	static my_map * collisionLibrary;
	static list<btCollisionShape*> * collisionSubShapes;

	bool physInit;
	btCollisionShape* colShape;
	string collisionType;
private:
};

#endif