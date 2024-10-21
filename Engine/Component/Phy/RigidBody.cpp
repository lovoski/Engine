#include "Component/Phy/RigidBody.hpp"

namespace aEngine {

RigidBody::RigidBody(EntityID id) : BaseComponent(id) {}

RigidBody::~RigidBody() {}

};

REGISTER_COMPONENT(aEngine, RigidBody);