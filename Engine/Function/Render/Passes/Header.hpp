#pragma once

#include "Function/Render/RenderPass.hpp"

namespace aEngine {

namespace Render {

extern const std::string outlineVS, outlineFS;
class OutlinePass : public BasePass {
public:
  OutlinePass();

  float OutlineWidth = 0.02f;
  // interpolate between OutlineColor and OutlineColorMap
  float OutlineWeight = 0.0f;
  glm::vec3 OutlineColor = glm::vec3(0.0f);

  Texture OutlineColorMap;

  void FinishPass() override;
  std::string getInspectorWindowName() override;

  template <typename Archive> void save(Archive &ar) const {
    ar(OutlineWidth, OutlineWeight, OutlineColor, OutlineColorMap.path);
  }
  template <typename Archive> void load(Archive &ar) {
    std::string outlineColorMapPath;
    ar(OutlineWidth, OutlineWeight, OutlineColor, outlineColorMapPath);
    // initTexture(outlineColorMapPath);
  }

protected:
  void initTexture(std::string path);
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

extern const std::string wireframeVS, wireframeFS;
class WireFramePass : public BasePass {
public:
  WireFramePass();

  float wireframeOffset = 0.001f;
  glm::vec3 wireFrameColor = glm::vec3(1.0f);

  void FinishPass() override;
  std::string getInspectorWindowName() override;

  template <typename Archive> void serialize(Archive &ar) {
    ar(wireframeOffset, wireFrameColor);
  }

private:
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

extern const std::string GBVMainVS, GBVMainFS;
class GBVMainPass : public BasePass {
public:
  GBVMainPass();

  // texture maps
  Texture base, ILM, SSS;

  // ramp
  float firstRampStart = 0.2, firstRampStop = 0.22;
  float rampOffset = 1.0f, rampShadowWeight = 1.0f;

  // rim light
  glm::vec3 rimLightColor = glm::vec3(0.8f);
  float rimLightWidth = 0.063f;
  float rimLightSmooth = 0.01f;

  // specular
  int specularGloss = 20;
  float specularWeight = 0.0f;

  // details
  float detailWeight = 0.3f;
  float innerLineWeight = 1.0f;
  Texture detail;

  std::string getInspectorWindowName() override;
  void FinishPass() override;

  template <typename Archive> void save(Archive &ar) const {
    ar(firstRampStart, firstRampStop, rampOffset, rampShadowWeight,
       rimLightColor, rimLightSmooth, rimLightWidth, specularGloss,
       specularWeight, detailWeight, innerLineWeight, base.path, ILM.path,
       SSS.path, detail.path);
  }
  template <typename Archive> void load(Archive &ar) {
    std::string p1, p2, p3, p4;
    ar(firstRampStart, firstRampStop, rampOffset, rampShadowWeight,
       rimLightColor, rimLightSmooth, rimLightWidth, specularGloss,
       specularWeight, detailWeight, innerLineWeight, p1, p2, p3, p4);
    initTexture(p1, p2, p3, p4);
  }

private:
  void initTexture(std::string p1, std::string p2, std::string p3,
                   std::string p4);
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

extern const std::string pbrVS, pbrFS;
class PBRPass : public BasePass {
public:
  PBRPass();

  float RoughnessFactor = 1.0f;
  float MetallicFactor = 1.0f;
  float AOFactor = 1.0f;
  glm::vec3 AlbedoFactor = glm::vec3(1.0f);
  glm::vec3 F0 = glm::vec3(0.04f);
  bool WithNormalMap = false;
  Texture Roughness, Metallic, AO, Albedo, Normal;

  std::string getInspectorWindowName() override;
  void FinishPass() override;

  template <typename Archive> void save(Archive &ar) const {
    ar(RoughnessFactor, MetallicFactor, AOFactor, AlbedoFactor, F0,
       WithNormalMap, Roughness.path, Metallic.path, AO.path, Albedo.path,
       Normal.path);
  }
  template <typename Archive> void load(Archive &ar) {
    std::string p1, p2, p3, p4, p5;
    ar(RoughnessFactor, MetallicFactor, AOFactor, AlbedoFactor, F0,
       WithNormalMap, p1, p2, p3, p4, p5);
    initTexture(p1, p2, p3, p4, p5);
  }

private:
  void initTexture(std::string p1, std::string p2, std::string p3,
                   std::string p4, std::string p5);
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

}; // namespace Render

}; // namespace aEngine