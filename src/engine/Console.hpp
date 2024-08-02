#pragma once

#include <imgui.h>

class AppLog {
public:
  ImGuiTextBuffer Buf;
  ImGuiTextFilter Filter;
  ImVector<int> LineOffsets; // Index to lines offset. We maintain this with
                             // AddLog() calls.
  bool AutoScroll;           // Keep scrolling if already at the bottom.

  static AppLog &Ref() {
    static AppLog reference;
    return reference;
  }

  AppLog() {
    AutoScroll = true;
    Clear();
  }

  void Clear();

  void Log(const char *fmt, ...) IM_FMTARGS(2);

  void Draw(const char *title, bool *p_open = NULL);

private:
  unsigned int counter = 0;

};

static AppLog &Console = AppLog::Ref();