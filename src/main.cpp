#include "basics.hpp"

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