#ifndef EVENTS_HPP
#define EVENTS_HPP

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

private:
  Events();
};

static Events &Event = Events::Ref();

#endif