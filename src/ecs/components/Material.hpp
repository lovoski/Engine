#pragma once

#include "Lights.hpp"
#include "Transform.hpp"
#include "ecs/ecs.hpp"
#include "resource/ResourceManager.hpp"
#include "resource/ResourceTypes.hpp"
#include "resource/Shader.hpp"

class BaseMaterial : public ECS::BaseComponent {
public:
  BaseMaterial() { SetShader(); }
  ~BaseMaterial() {}

  void SetShader(string vertShaderPath = REPO_SOURCE_DIR
                 "/src/shaders/default/base.vert",
                 string fragShaderPath = REPO_SOURCE_DIR
                 "/src/shaders/default/base.frag") {
    shader = Resource::RManager.GetShader(vertShaderPath, fragShaderPath);
    VertShader = vertShaderPath;
    FragShader = fragShaderPath;
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
    unsigned int diffuseNr = 0, specularNr = 0, normalNr = 0, heightNr = 0;
    unsigned int imageTexCounter = 0;
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
      else if (name == "imageTex") // custom loaded image texture
        number = std::to_string(imageTexCounter++);

      // now set the sampler to the correct texture unit
      glUniform1i(glGetUniformLocation(shader->ID, (name + number).c_str()), i);
      // and finally bind the texture
      glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
  }

  void SetFixedVariables() {
    shader->SetVec3("Albedo", vec3(Albedo[0], Albedo[1], Albedo[2]));
    shader->SetFloat("Ambient", Ambient);
  }

  void SetBaseLights(vector<BaseLight> &lights) {
    unsigned int dirLightCounter = 0;
    unsigned int pointLightCounter = 0;
    unsigned int spotLightCounter = 0;
    // set the properties of different lights
    for (auto &light : lights) {
      if (light.Type == BaseLight::DIRECTIONAL_LIGHT) {
        string lightDirName = "dLightDir" + std::to_string(dirLightCounter);
        string lightColorName = "dLightColor" + std::to_string(dirLightCounter);
        shader->SetVec3(lightDirName, light.LightDir);
        shader->SetVec3(lightColorName, vec3(light.LightColor[0], light.LightColor[1], light.LightColor[2]));
        dirLightCounter++;
      } else if (light.Type == BaseLight::POINT_LIGHT) {
        string lightPosName = "pLightPos" + std::to_string(pointLightCounter);
        string lightColorName =
            "pLightColor" + std::to_string(pointLightCounter);
        shader->SetVec3(
            lightPosName,
            ECS::EManager.GetComponent<Transform>(light.GetID()).Position);
        shader->SetVec3(lightColorName, vec3(light.LightColor[0], light.LightColor[1], light.LightColor[2]));
        pointLightCounter++;
      } else if (light.Type == BaseLight::SPOT_LIGHT) {
      }
    }
    if (dirLightCounter == 0) {
      Console.Log("[error]: At least one directional light needed for the default shader\n");
    }
  }

  float Albedo[3] = {1.0f, 1.0f, 1.0f};
  float Ambient = 0.1f;


  Resource::Shader *shader = nullptr;
  vector<Resource::Texture> textures;

  string VertShader, FragShader;
};