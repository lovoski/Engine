#include "Engine.hpp"

using namespace std;
using namespace aEngine;

int main() {
  Engine engine(1920, 1080);
  engine.Start();
  while (engine.Run()) {
    engine.Update();

    engine.RenderBegin();
    engine.RenderEnd();
  }
  return 0;
}