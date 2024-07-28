#pragma once

#include "global.hpp"

class Action {
public:
  enum ActionType {
    MOUSE_SCROLL
  };

  Action(ActionType type, void *data) {
    Type = type;
    Data = data;
  }

  ActionType Type;
  void *Data = nullptr;
};

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

  vector<Action> actions;

  // don't call these variables
  vec2 _currentScrollOffset = vec2(0.0f);

private:

  Events();
};

static Events &Event = Events::Ref();
