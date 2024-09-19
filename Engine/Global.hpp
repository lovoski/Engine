/**
 * Don't change the order of include in this file,
 * always include glad before glfw, include asio before glad
 * and other platform specific headers.
 */
#pragma once

#include <asio.hpp>

#include <tinyfiledialogs.h>

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

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stb_image.h>

#include <filesystem>
namespace fs = std::filesystem;

#include <loguru.hpp>

// imgui based libraries
#include <implot.h>
#include <ImGuizmo.h>

#include <EngineConfig.h>

namespace glm {
  template<class Archive> void serialize(Archive& archive, glm::vec2& v) { archive(v.x, v.y); }
  template<class Archive> void serialize(Archive& archive, glm::vec3& v) { archive(v.x, v.y, v.z); }
  template<class Archive> void serialize(Archive& archive, glm::vec4& v) { archive(v.x, v.y, v.z, v.w); }
  template<class Archive> void serialize(Archive& archive, glm::ivec2& v) { archive(v.x, v.y); }
  template<class Archive> void serialize(Archive& archive, glm::ivec3& v) { archive(v.x, v.y, v.z); }
  template<class Archive> void serialize(Archive& archive, glm::ivec4& v) { archive(v.x, v.y, v.z, v.w); }
  template<class Archive> void serialize(Archive& archive, glm::uvec2& v) { archive(v.x, v.y); }
  template<class Archive> void serialize(Archive& archive, glm::uvec3& v) { archive(v.x, v.y, v.z); }
  template<class Archive> void serialize(Archive& archive, glm::uvec4& v) { archive(v.x, v.y, v.z, v.w); }
  template<class Archive> void serialize(Archive& archive, glm::dvec2& v) { archive(v.x, v.y); }
  template<class Archive> void serialize(Archive& archive, glm::dvec3& v) { archive(v.x, v.y, v.z); }
  template<class Archive> void serialize(Archive& archive, glm::dvec4& v) { archive(v.x, v.y, v.z, v.w); }

  template<class Archive> void serialize(Archive& archive, glm::mat2& m) { archive(m[0], m[1]); }
  template<class Archive> void serialize(Archive& archive, glm::dmat2& m) { archive(m[0], m[1]); }
  template<class Archive> void serialize(Archive& archive, glm::mat3& m) { archive(m[0], m[1], m[2]); }
  template<class Archive> void serialize(Archive& archive, glm::mat4& m) { archive(m[0], m[1], m[2], m[3]); }
  template<class Archive> void serialize(Archive& archive, glm::dmat4& m) { archive(m[0], m[1], m[2], m[3]); }

  template<class Archive> void serialize(Archive& archive, glm::quat& q) { archive(q.x, q.y, q.z, q.w); }
  template<class Archive> void serialize(Archive& archive, glm::dquat& q) { archive(q.x, q.y, q.z, q.w); }
}