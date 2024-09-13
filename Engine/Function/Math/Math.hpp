#include "Global.hpp"

namespace aEngine {

namespace Math {

glm::quat FromToRotation(glm::vec3 from, glm::vec3 to);

glm::quat LookAtRotation(glm::vec3 forward, glm::vec3 up);

glm::vec3 FaceNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);

void DecomposeTransform(const glm::mat4 &transform, glm::vec3 &outPosition,
                        glm::quat &outRotation, glm::vec3 &outScale);

};

};