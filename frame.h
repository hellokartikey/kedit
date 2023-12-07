#ifndef KEDIT_FRAME_H
#define KEDIT_FRAME_H

#include "common.h"
#include "point.h"

class kedit;

class frame {
 public:
  frame();

  auto init(kedit* editor) -> void;
  auto resize(point tl, point br) -> void;

  auto get_editor() -> kedit*;
  auto set_editor(kedit* editor) -> void;

  auto get_curs() const -> const point;
  auto set_curs(point p) -> void;

  auto get_coord() -> point;

  auto get_tl() -> point;
  auto set_tl(point p) -> void;

  auto get_br() -> point;
  auto set_br(point p) -> void;

  auto curs_move() -> void;
  auto curs_dec_x() -> void;
  auto curs_dec_y() -> void;
  auto curs_inc_x() -> void;
  auto curs_inc_y() -> void;
  auto curs_restrict() -> void;
  auto curs_home() -> void;
  auto curs_end() -> void;

  virtual auto render() -> void {}

 protected:
  point tl;
  point br;

  point size;

  point curs;

  kedit* editor;
};

#endif
