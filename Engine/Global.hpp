#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_DEFINE_MATH_OPERATORS
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

#include <stb_image.h>

#include <DDraw.hpp>

#include <filesystem>
namespace fs = std::filesystem;

#include <ImGuizmo.h>

#include <ImGuiFileDialog.h>

#include <NlohmannJson.hpp>

using Json = nlohmann::json;

class Logger {
public:
  ImGuiTextBuffer Buf;
  ImGuiTextFilter Filter;
  ImVector<int> LineOffsets; // Index to lines offset. We maintain this with
                             // AddLog() calls.
  bool AutoScroll;           // Keep scrolling if already at the bottom.

  static Logger &Ref() {
    static Logger reference;
    return reference;
  }

  Logger() {
    AutoScroll = true;
    Clear();
  }

  void Clear();

  void Log(const char *fmt, ...) IM_FMTARGS(2);

  void Draw(const char *title, bool *p_open = NULL);

private:
  unsigned int counter = 0;

};

static Logger &Console = Logger::Ref();

// Register some serializable class
NLOHMANN_JSON_NAMESPACE_BEGIN
template <> struct adl_serializer<glm::vec2> {
  static void to_json(json &j, const glm::vec2 &v) {
    j["serializeType"] = "glm::vec2";
    j["x"] = v.x;
    j["y"] = v.y;
  }
  static void from_json(const json &j, glm::vec2 &v) {
    v.x = j["x"];
    v.y = j["y"];
  }
};
template <> struct adl_serializer<glm::vec3> {
  static void to_json(json &j, const glm::vec3 &v) {
    j["serializeType"] = "vec3";
    j["x"] = v.x;
    j["y"] = v.y;
    j["z"] = v.z;
  }
  static void from_json(const json &j, glm::vec3 &v) {
    v.x = j["x"];
    v.y = j["y"];
    v.z = j["z"];
  }
};
template <> struct adl_serializer<glm::vec4> {
  static void to_json(json &j, const glm::vec4 &v) {
    j["serializeType"] = "vec4";
    j["x"] = v.x;
    j["y"] = v.y;
    j["z"] = v.z;
    j["w"] = v.w;
  }
  static void from_json(const json &j, glm::vec4 &v) {
    v.x = j["x"];
    v.y = j["y"];
    v.z = j["z"];
    v.w = j["w"];
  }
};
template <> struct adl_serializer<glm::quat> {
  static void to_json(json &j, const glm::quat &q) {
    j["serializeType"] = "quat";
    j["w"] = q.w;
    j["x"] = q.x;
    j["y"] = q.y;
    j["z"] = q.z;
  }
  static void from_json(const json &j, glm::quat &q) {
    q.w = j["w"];
    q.x = j["x"];
    q.y = j["y"];
    q.z = j["z"];
  }
};
NLOHMANN_JSON_NAMESPACE_END
