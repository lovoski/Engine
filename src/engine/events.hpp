#pragma once

#include "global.hpp"

class Events {
public:
  Events(const Events &) = delete;
  ~Events();
  Events &operator=(const Events &) = delete;

  static Events &Ref() {
    static Events reference;
    return reference;
  }

  void Poll();
  void Initialize();

  int GetKey(int key);
  int GetMouseButton(int button);

  vec2 MouseCurrentPosition = vec2(0.0f);
  vec2 MouseScrollOffset = vec2(0.0f);

private:

  Events();
};

static Events &Event = Events::Ref();
