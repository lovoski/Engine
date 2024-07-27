#pragma once

#include "ecs/ecs.hpp"
#include "resource/ResourceManager.hpp"
#include "resource/ResourceTypes.hpp"
#include "resource/Shader.hpp"

class BaseMaterial : public ECS::BaseComponent {
public:
  BaseMaterial() {}
  ~BaseMaterial() {}

  void SetShader(string vertShaderPath = REPO_SOURCE_DIR
                 "/src/shaders/default/pbr.vert",
                 string fragShaderPath = REPO_SOURCE_DIR
                 "/src/shaders/default/pbr.frag") {
    shader = Resource::ResourceManager.GetShader(vertShaderPath, fragShaderPath);
  }
  Resource::Shader *GetShader() {
    if (this->shader == nullptr) {
      Console.Log("[error]: material has no valid shader\n");
    }
    return this->shader;
  }

  void SetTextures(vector<Resource::Texture> textures) {
    this->textures = textures;
  }

  void ActivateTextures() {
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < textures.size(); i++) {
      glActiveTexture(GL_TEXTURE0 +
                      i); // active proper texture unit before binding
      // retrieve texture number (the N in diffuse_textureN)
      string number;
      string name = textures[i].type;
      if (name == "texture_diffuse")
        number = std::to_string(diffuseNr++);
      else if (name == "texture_specular")
        number =
            std::to_string(specularNr++); // transfer unsigned int to string
      else if (name == "texture_normal")
        number = std::to_string(normalNr++); // transfer unsigned int to string
      else if (name == "texture_height")
        number = std::to_string(heightNr++); // transfer unsigned int to string

      // now set the sampler to the correct texture unit
      glUniform1i(glGetUniformLocation(shader->ID, (name + number).c_str()), i);
      // and finally bind the texture
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
  }

protected:
  vec3 albedo = vec3(1.0f);
  Resource::Shader *shader = nullptr;
  vector<Resource::Texture> textures;
};