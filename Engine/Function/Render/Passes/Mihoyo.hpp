/**
 * Shaders for mihoyo characters.
 */
#pragma once

#include "Function/Render/RenderPass.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine::Render {

class StarRailCharacter : public BasePass {
public:
  StarRailCharacter();

  std::string getInspectorWindowName() override { return "Honkai Railway"; }
  void FinishPass() override;

  template <typename Archive> void save(Archive &ar) const {}
  template <typename Archive> void load(Archive &ar) {}

private:
  // void initTextures(std::string d, std::string m, std::string n);
  unsigned int nullTex, whiteTex;
  void initTextures(std::string d, std::string m, std::string n);
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

}; // namespace aEngine::Render
