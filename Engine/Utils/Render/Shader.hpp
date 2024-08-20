#pragma once

#include "Global.hpp"
#include <ShaderInclude.hpp>

namespace aEngine {

namespace Render {

class Shader {
public:
  std::string identifier;
  unsigned int ID = 0;
  // do nothing at the constructor
  Shader() {}
  ~Shader() { glDeleteProgram(ID); }

  // Load shader code directly, create and link program
  bool LoadAndRecompileShaderSource(std::string vss, std::string fss,
                                    std::string gss = "none") {
    unsigned int vertex, fragment;
    // vertex shader
    const char *vShaderCode = vss.c_str();
    const char *fShaderCode = fss.c_str();
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // if geometry shader is given, compile geometry shader
    unsigned int geometry;
    if (gss != "none") {
      const char *gShaderCode = gss.c_str();
      geometry = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(geometry, 1, &gShaderCode, NULL);
      glCompileShader(geometry);
      checkCompileErrors(geometry, "GEOMETRY");
    }
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (gss != "none")
      glAttachShader(ID, geometry);
    // linking the shader is a time consuming process
    // this should be avoid between frames in all cost
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer
    // necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (gss != "none")
      glDeleteShader(geometry);
    return true;
  }

  // Load shader from path, compile and link them into a program
  bool LoadAndRecompileShader(std::string vsp, std::string fsp,
                              std::string gsp = "none") {
    // delete previous program if exists
    if (ID != 0) {
      // free old shader if there's any
      glDeleteProgram(ID);
    }
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode = "none";
    try {
      vertexCode = ShaderInclude::load(vsp, "//include");
      fragmentCode = ShaderInclude::load(fsp, "//include");
      // if geometry shader path is present, also load a geometry shader
      if (gsp != "none") {
        geometryCode = ShaderInclude::load(gsp, "//include");
      }
    } catch (std::ifstream::failure &e) {
      Console.Log(
          "[error]: failed to load shader from path, %s\n",
          e.what());
      return false;
    }
    return LoadAndRecompileShaderSource(vertexCode, fragmentCode, geometryCode);
  }

  void Use() { glUseProgram(ID); }
  void SetBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }
  void SetInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }
  void SetFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }
  void SetVec2(const std::string &name, glm::vec2 value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void SetVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
  }
  void SetVec3(const std::string &name, glm::vec3 value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void SetVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
  }
  void SetVec4(const std::string &name, glm::vec4 value) const {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  void SetVec4(const std::string &name, float x, float y, float z, float w) {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
  }
  void SetMat2(const std::string &name, glm::mat2 mat) const {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }
  void SetMat3(const std::string &name, glm::mat3 mat) const {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }
  void SetMat4(const std::string &name, glm::mat4 mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }

private:
  void checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        Console.Log("[error]::SHADER_COMPILATION_ERROR of type: %s\n%s\n",
                    type.c_str(), infoLog);
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        Console.Log("[error]::PROGRAM_LINKING_ERROR of type: %s\n%s\n",
                    type.c_str(), infoLog);
      }
    }
  }
};

const std::string errorVS = R"(
#version 460 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 ModelToWorldPoint;
uniform mat4 View;
uniform mat4 Projection;
void main() {
  gl_Position = Projection * View * ModelToWorldPoint * aPos;
}
)";
const std::string errorFS = R"(
#version 460 core
out vec4 FragColor;
void main() {
  FragColor = vec4(0.702, 0.2314, 0.7725, 1.0);
}
)";

// the default diffuse shader
const std::string diffuseVS = R"(
#version 460 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec2 aTexCoord;
out vec3 normal;
// transform point from model space to world space
uniform mat4 ModelToWorldPoint;
// transform vector from model space to world space
uniform mat3 ModelToWorldDir;
// transform world space to camera space
uniform mat4 View;
// transform camera space to screen
uniform mat4 Projection;
void main() {
  normal = ModelToWorldDir * vec3(aNormal);
  gl_Position = Projection * View * ModelToWorldPoint * aPos;
}
)";

const std::string diffuseFS = R"(
#version 430 core
uniform vec3 Albedo;
uniform vec3 dLightDir0;
uniform vec3 dLightColor0;
uniform float Ambient;
in vec3 normal;
out vec4 FragColor;
void main() {
  // ambient
  vec3 ambient = Ambient * dLightColor0;
  // diffuse
  vec3 Normal = normalize(normal);
  vec3 LightDir = -dLightDir0;
  float lambert = (dot(Normal, LightDir) + 1.0) * 0.5;
  vec3 diffuse = lambert * dLightColor0 * Albedo;
  vec3 result = ambient + diffuse;
  FragColor = vec4(result, 1.0);
}
)";

}; // namespace Render

}; // namespace aEngine