#pragma once

#include "EngineConfig.h"
#include "ResourceTypes.hpp"
#include "Shader.hpp"
#include "ecs/systems/render/Mesh.hpp"

namespace Resource {

class tResourceManager {
public:
  tResourceManager() {}
  tResourceManager(tResourceManager &) = delete;
  const tResourceManager &operator=(tResourceManager &) = delete;
  ~tResourceManager() {}

  static tResourceManager &Ref() {
    static tResourceManager reference;
    return reference;
  }

  // Graphics::Mesh GetPrimitive(PRIMITIVE_TYPE pType);

  vector<Graphics::Mesh> GetModel(string);

  Shader *GetShader(string vertShaderPath, string fragShaderPath);

private:
  // stores all the loaded textures
  vector<Texture> texturesLoaded;
  // stores all the loaded shaders
  vector<Shader> shaderLoaded;

  unsigned int textureFromFile(string texturePath, bool gamma = false);

  vector<Texture> loadMaterialTextures(aiMaterial *, aiTextureType, string);

  Graphics::Mesh processMesh(aiMesh *, const aiScene *);

  void processNode(aiNode *, const aiScene *, vector<Graphics::Mesh> &);
};

static tResourceManager &ResourceManager = tResourceManager::Ref();

}; // namespace Resource