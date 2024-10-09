#include "Editor.hpp"

int main() {
  Editor editor(1920, 1080);
  editor.Start();

  // the main loop
  editor.Run(false);

  editor.Shutdown();
  return 0;
}