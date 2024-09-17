#pragma once

#include "Global.hpp"
#include "Function/AssetsType.hpp"

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
                                    std::string gss = "none");

  // Load shader from path, compile and link them into a program
  bool LoadAndRecompileShader(std::string vsp, std::string fsp,
                              std::string gsp = "none");

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

  // Activate the texture at a opengl bind point, if the texture is
  // ::null_texture, pure white texture will be activated instead.
  bool SetTexture2D(Texture &texture, std::string name, int slot);

  bool SetCubeMap(std::string name, unsigned int cubemapID, int slot);

private:
  void checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        LOG_F(ERROR, "SHADER_COMPILATION_ERROR of type: %s\n %s", type.c_str(),
              infoLog);
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        LOG_F(ERROR, "PROGRAM_LINKING_ERROR of type: %s\n %s", type.c_str(),
              infoLog);
      }
    }
  }
};

}; // namespace Render

}; // namespace aEngine