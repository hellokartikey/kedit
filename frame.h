#ifndef KEDIT_FRAME_H
#define KEDIT_FRAME_H

#include "point.h"

class kedit;

class frame {
 public:
  frame();
  frame(kedit* editor);

  virtual auto render() -> void;

  auto set_editor(kedit* editor) -> void;
  auto get_editor() -> kedit*;

 private:
  point tl;
  point br;

  kedit* editor;
};

#endif
