#include "basics.hpp"

#include "global.hpp"
#include "ecs/components/Transform.hpp"
#include "ecs/components/Camera.hpp"

int main() {
  Core.Initialize();
  Timer.Initialize();
  Event.Initialize();

  while (Core.Run()) {
    Timer.Tick();
    Event.Poll();
    Core.Update();
  }

  return 0;
}