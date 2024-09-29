/**
 * Don't change the order of include in this file,
 * always include glad before glfw, include asio before glad
 * and other platform specific headers.
 */
#pragma once

#include <tinyfiledialogs.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>

#include <miniaudio.h>

#include <filesystem>
namespace fs = std::filesystem;

#include <loguru.hpp>

// imgui based libraries
#include <ImGuizmo.h>
#include <implot.h>

#include <EngineConfig.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/queue.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/stack.hpp>
#include <boost/serialization/base_object.hpp>

namespace boost {
namespace serialization {

template <class Archive>
void serialize(Archive &ar, glm::vec2 &v, const unsigned int version) {
  ar &v.x;
  ar &v.y;
}
template <class Archive>
void serialize(Archive &ar, glm::vec3 &v, const unsigned int version) {
  ar &v.x;
  ar &v.y;
  ar &v.z;
}
template <class Archive>
void serialize(Archive &ar, glm::vec4 &v, const unsigned int version) {
  ar &v.x;
  ar &v.y;
  ar &v.z;
  ar &v.w;
}
template <class Archive>
void serialize(Archive &ar, glm::ivec2 &v, const unsigned int version) {
  ar &v.x;
  ar &v.y;
}
template <class Archive>
void serialize(Archive &ar, glm::ivec3 &v, const unsigned int version) {
  ar &v.x;
  ar &v.y;
  ar &v.z;
}
template <class Archive>
void serialize(Archive &ar, glm::ivec4 &v, const unsigned int version) {
  ar &v.x;
  ar &v.y;
  ar &v.z;
  ar &v.w;
}
template <class Archive>
void serialize(Archive &ar, glm::quat &q, const unsigned int version) {
  ar &q.x;
  ar &q.y;
  ar &q.z;
  ar &q.w;
}
template <class Archive>
void serialize(Archive &ar, glm::dquat &q, const unsigned int version) {
  ar &q.x;
  ar &q.y;
  ar &q.z;
  ar &q.w;
}
template <class Archive>
void serialize(Archive &ar, glm::mat2 &m, const unsigned int version) {
  ar &m[0];
  ar &m[1];
}
template <class Archive>
void serialize(Archive &ar, glm::dmat2 &m, const unsigned int version) {
  ar &m[0];
  ar &m[1];
}
template <class Archive>
void serialize(Archive &ar, glm::mat3 &m, const unsigned int version) {
  ar &m[0];
  ar &m[1];
  ar &m[2];
}
template <class Archive>
void serialize(Archive &ar, glm::dmat3 &m, const unsigned int version) {
  ar &m[0];
  ar &m[1];
  ar &m[2];
}
template <class Archive>
void serialize(Archive &ar, glm::mat4 &m, const unsigned int version) {
  ar &m[0];
  ar &m[1];
  ar &m[2];
  ar &m[3];
}
template <class Archive>
void serialize(Archive &ar, glm::dmat4 &m, const unsigned int version) {
  ar &m[0];
  ar &m[1];
  ar &m[2];
  ar &m[3];
}

} // namespace serialization
} // namespace boost
