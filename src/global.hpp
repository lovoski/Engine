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

#include <filesystem>
namespace fs = std::filesystem;

#include <ImGuizmo.h>

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

#include "utils/json.hpp"

using Json = nlohmann::json;

// Register some serializable class
NLOHMANN_JSON_NAMESPACE_BEGIN
template <> struct adl_serializer<vec2> {
  static void to_json(json &j, const vec2 &v) {
    j["serializeType"] = "vec2";
    j["x"] = v.x;
    j["y"] = v.y;
  }
  static void from_json(const json &j, vec2 &v) {
    v.x = j["x"];
    v.y = j["y"];
  }
};
template <> struct adl_serializer<vec3> {
  static void to_json(json &j, const vec3 &v) {
    j["serializeType"] = "vec3";
    j["x"] = v.x;
    j["y"] = v.y;
    j["z"] = v.z;
  }
  static void from_json(const json &j, vec3 &v) {
    v.x = j["x"];
    v.y = j["y"];
    v.z = j["z"];
  }
};
template <> struct adl_serializer<vec4> {
  static void to_json(json &j, const vec4 &v) {
    j["serializeType"] = "vec4";
    j["x"] = v.x;
    j["y"] = v.y;
    j["z"] = v.z;
    j["w"] = v.w;
  }
  static void from_json(const json &j, vec4 &v) {
    v.x = j["x"];
    v.y = j["y"];
    v.z = j["z"];
    v.w = j["w"];
  }
};
template <> struct adl_serializer<quat> {
  static void to_json(json &j, const quat &q) {
    j["serializeType"] = "quat";
    j["w"] = q.w;
    j["x"] = q.x;
    j["y"] = q.y;
    j["z"] = q.z;
  }
  static void from_json(const json &j, quat &q) {
    q.w = j["w"];
    q.x = j["x"];
    q.y = j["y"];
    q.z = j["z"];
  }
};
NLOHMANN_JSON_NAMESPACE_END
