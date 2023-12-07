#include "frame.h"

#include "kedit.h"

frame::frame() : editor(nullptr) {}

auto frame::init(kedit* editor, point tl, point br) -> void {
  this->editor = editor;
  this->tl = tl;
  this->br = br;
  this->size = br - tl;
}

auto frame::set_editor(kedit* editor) -> void { this->editor = editor; }

auto frame::get_editor() -> kedit* { return editor; }

auto frame::get_curs() -> point { return curs; }

auto frame::set_curs(point p) -> void { curs = p; }

auto frame::get_coord() -> point { return tl + curs; }

auto frame::get_tl() -> point { return tl; }

auto frame::set_tl(point p) -> void { tl = p; }

auto frame::get_br() -> point { return br; }

auto frame::set_br(point p) -> void { br = p; }

auto frame::curs_move() -> void {
  auto tmp = tl + curs;
  move(tmp.y, tmp.x);
}

auto frame::curs_dec_x() -> void {
  if ( curs.x == 0 and curs.y == 0 ) return;

  if ( curs.x == 0 ) {
    curs.x = size.x;
    curs_dec_y();
    return;
  }

  curs.x--;
}

auto frame::curs_dec_y() -> void {
  if ( curs.y == size.y ) return;

  curs.y--;
}

auto frame::curs_inc_x() -> void {
  if ( curs.x == size.x and curs.y == size.y ) return;

  curs.x++;

  if ( curs.x > size.x ) {
    curs.x = 0;
    curs_inc_y();
  }
}

auto frame::curs_inc_y() -> void {
  if ( curs.y == size.y ) return;

  curs.y++;
}
