#include "Global.hpp"

namespace aEngine {

namespace Math {

glm::quat FromToRotation(glm::vec3 from, glm::vec3 to);

glm::quat LookAtRotation(glm::vec3 forward, glm::vec3 up);

void DecomposeTransform(const glm::mat4 &transform, glm::vec3 &outPosition,
                        glm::quat &outRotation, glm::vec3 &outScale);

};

};