#include "Component/Phy/RigidBody.hpp"

namespace aEngine {

RigidBody::RigidBody(EntityID id) : BaseComponent(id) {}

RigidBody::~RigidBody() {}

}; // namespace aEngine

REGISTER_COMPONENT(aEngine, RigidBody);