#include <btBulletDynamicsCommon.h>

#include <iostream>

int main()
{
  // Specify the dynamic AABB tree broadphase algorithm to be used to work out what objects
  // to test collision for.
  btBroadphaseInterface* broadphase = new btDbvtBroadphase();

  // The collision configuration allows you to fine tune the algorithms used
  // for the full (not broadphase) collision detection. Here be dragons!
  btDefaultCollisionConfiguration* collisionConfiguration =
    new btDefaultCollisionConfiguration();

  btCollisionDispatcher* dispatcher =
    new btCollisionDispatcher(collisionConfiguration);

  // We also need a "solver". This is what causes the objects to interact
  // properly, taking into account gravity, game logic supplied forces,
  // collisions, and hinge constraints. It does a good job as long as you
  // don't push it to extremes, and is one of the bottlenecks in any high
  // performance simulation. There are parallel versions available for some
  // threading models.
  btSequentialImpulseConstraintSolver* solver =
    new btSequentialImpulseConstraintSolver;

  // Now, we can finally instantiate the dynamics world.
  btDiscreteDynamicsWorld* dynamicsWorld =
    new btDiscreteDynamicsWorld(dispatcher, broadphase, solver,
    collisionConfiguration);

  // Set the gravity. We have chosen the Y axis to be "up".
  dynamicsWorld->setGravity(btVector3(0, -10, 0));

  // In this demonstration, we will place a ground plane running through
  // the origin.
  btCollisionShape* groundShape = new btStaticPlaneShape(
    btVector3(0, 1, 0), 1);

  // The shape that we will let fall from the sky is a sphere with a radius
  // of 1 metre. 
  btCollisionShape* fallShape = new btSphereShape(1);

  // Instantiate the ground. Its orientation is the identity, Bullet quaternions
  // are specified in x,y,z,w form. The position is 1 metre below the ground,
  // which compensates the 1m offset we had to put into the shape itself.
  btDefaultMotionState* groundMotionState = new btDefaultMotionState(
    btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));

  // The first and last parameters of the following constructor are the mass and
  // inertia of the ground. Since the ground is static, we represent this by
  // filling these values with zeros. Bullet considers passing a mass of zero
  // equivalent to making a body with infinite mass - it is immovable.
  btRigidBody::btRigidBodyConstructionInfo
    groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));

  btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);

  // Add the ground to the world.
  dynamicsWorld->addRigidBody(groundRigidBody);

  // Adding the falling sphere is very similar. We will place it 50m above the
  // ground.
  btDefaultMotionState* fallMotionState =
    new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1),
    btVector3(0, 50, 0)));

  // Since it's dynamic we will give it a mass of 1kg. I can't remember how to
  // calculate the inertia of a sphere, but that doesn't matter because Bullet
  // provides a utility function.
  btScalar mass = 1;
  btVector3 fallInertia(0, 0, 0);
  fallShape->calculateLocalInertia(mass, fallInertia);

  // Construct the rigid body just like before, and add it to the world.
  btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,
   fallMotionState, fallShape, fallInertia);

  btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
  dynamicsWorld->addRigidBody(fallRigidBody);

  // This is where the fun begins. Step the simulation 300 times, at an interval
  // of 60hz. This will give the sphere enough time to hit the ground under the
  // influence of gravity. Each step, we will print out its height above the
  // ground.
  for(int i = 0; i < 300; i++)
  {
    dynamicsWorld->stepSimulation(1 / 60.f, 10);

    btTransform trans;
    fallRigidBody->getMotionState()->getWorldTransform(trans);

    std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
  }

  // Clean up behind ourselves like good little programmers. Note that this is
  // INCORRECT. We should be using unique_ptr<T> or shared_ptr<T> since we don't
  // know if any of the above functions throw an exception.
  dynamicsWorld->removeRigidBody(fallRigidBody);
  delete fallRigidBody->getMotionState();
  delete fallRigidBody;

  dynamicsWorld->removeRigidBody(groundRigidBody);
  delete groundRigidBody->getMotionState();
  delete groundRigidBody;

  delete fallShape;
  delete groundShape;

  delete dynamicsWorld;
  delete solver;
  delete collisionConfiguration;
  delete dispatcher;
  delete broadphase;

  return 0;
}
