#include "Function/Render/Passes/Mihoyo.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/Passes/Header.hpp"

namespace aEngine::Render {

// shader for characters from: https://github.com/m4urlclo0/ZZZ-Assets

StarRailCharacter::StarRailCharacter() {
  shader = Loader.GetShader(ASSETS_PATH "/shaders/starrail/character.vs",
                            ASSETS_PATH "/shaders/starrail/character.fs");
  initTextures("::null_texture", "::null_texture", "::null_texture");
}
void StarRailCharacter::BeforePass() { glDisable(GL_CULL_FACE); }
void StarRailCharacter::FinishPass() {}
void StarRailCharacter::DrawInspectorGUI() {
  if (ImGui::Button("Reload Shader", {-1, 30}))
    shader = Loader.GetShader(ASSETS_PATH "/shaders/starrail/character.vs",
                              ASSETS_PATH "/shaders/starrail/character.fs");
}

void StarRailCharacter::initTextures(std::string d, std::string m,
                                    std::string n) {
  nullTex = Loader.GetTexture("::null_texture")->id;
  whiteTex = Loader.GetTexture("::white_texture")->id;
}

}; // namespace aEngine::Render

REGISTER_RENDER_PASS_SL(aEngine::Render, StarRailCharacter)