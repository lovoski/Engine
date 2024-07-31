#pragma once


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <limits>
#include <stack>
#include <tuple>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <stb_image.h>

#include "engine/Console.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::queue;
using std::stack;

using glm::mat3;
using glm::mat4;
using glm::quat;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::cross;
using glm::dot;
using glm::normalize;

#define MAX_FLOAT std::numeric_limits<float>::max()

inline std::ostream &operator<<(std::ostream &out, vec3 v) {
  out << "x:" << v.x << ",y:" << v.y << ",z:" << v.z;
  return out;
}