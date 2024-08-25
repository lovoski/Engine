#pragma once

#include "Global.hpp"

namespace aEngine {

class ComputeShader {
public:
  // Shader Program ID
  GLuint ID;

  std::string identifier;

  // Constructor reads and builds the compute shader
  ComputeShader(const std::string computeCode) {
    const char *cShaderCode = computeCode.c_str();
    // Compile shaders
    GLuint compute;
    GLint success;
    GLchar infoLog[1024];

    // Compute Shader
    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &cShaderCode, nullptr);
    glCompileShader(compute);
    checkCompileErrors(compute, "COMPUTE");

    // Shader Program
    ID = glCreateProgram();
    glAttachShader(ID, compute);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // Delete the shader as it's linked into our program now and no longer
    // necessary
    glDeleteShader(compute);
  }

  ~ComputeShader() { glDeleteProgram(ID); }

  // Use the program
  void Use() { glUseProgram(ID); }

  // Dispatch the compute shader
  void Dispatch(GLuint numGroupsX, GLuint numGroupsY, GLuint numGroupsZ) {
    glUseProgram(ID);
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
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

}; // namespace aEngine