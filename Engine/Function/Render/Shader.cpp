#include "Function/Render/Shader.hpp"
#include "Function/AssetsLoader.hpp"

#include <ShaderInclude.hpp>

namespace aEngine {

namespace Render {

// Load shader from path, compile and link them into a program
bool Shader::LoadAndRecompileShader(std::string vsp, std::string fsp,
                                    std::string gsp) {
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
    LOG_F(ERROR, "failed to load shader from path, %s", e.what());
    return false;
  }
  return LoadAndRecompileShaderSource(vertexCode, fragmentCode, geometryCode);
}

// Load shader code directly, create and link program
bool Shader::LoadAndRecompileShaderSource(std::string vss, std::string fss,
                                          std::string gss) {
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

bool Shader::SetTexture2D(Texture &texture, std::string name, int slot) {
  Use(); // activate the shader
  static Texture pureWhite = *Loader.GetTexture("::white_texture");
  auto activateTexId = texture.id;
  if (texture.path == "::null_texture") {
    activateTexId = pureWhite.id;
  }
  glActiveTexture(GL_TEXTURE0 + slot);
  int location = glGetUniformLocation(ID, name.c_str());
  if (location == -1) {
    // LOG_F(WARNING, "location for %s not valid", name.c_str());
    return false;
  }
  glUniform1i(location, slot);
  glBindTexture(GL_TEXTURE_2D, activateTexId);
  return true;
}

bool Shader::SetCubeMap(std::string name, unsigned int cubemapID, int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);
  int location = glGetUniformLocation(ID, name.c_str());
  if (location == -1)
    return false;
  glUniform1i(location, slot);
  return true;
}

}; // namespace Render

}; // namespace aEngine