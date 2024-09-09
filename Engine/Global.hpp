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
