#include "status.h"

#include <fmt/core.h>

#include "common.h"
#include "kedit.h"

auto status::render() -> void {
  // Set Black on White
  attrset(COLOR_PAIR(BLACK_WHITE));

  clear();
  begin();

  auto l_str = std::string{};
  auto r_str = std::string{};

  // Command
  if ( editor->get_command().size() ) {
    l_str += editor->get_command() + " ";
  }

  // Flashed Message
  if ( editor->get_flash().size() ) {
    l_str += editor->get_flash() + " ";
  }

  // Cursor Position
  auto edit_curs = editor->get_curs();
  r_str += fmt::format("[{}, {}] ", edit_curs.x, edit_curs.y);

  // Mode
  switch ( editor->get_mode() ) {
  case DEFAULT:
    r_str += "DEFAULT";
    break;
  case COMMAND:
    r_str += "COMMAND";
    break;
  case INSERT:
    r_str += "INSERT";
    break;
  default:
    break;
  }

  // Padding
  auto padding = std::string(size.x - l_str.size() - r_str.size(), ' ');

  // Printing
  auto status_bar = l_str + padding + r_str;
  printw("%s", status_bar.c_str());

  attrset(COLOR_PAIR(WHITE_BLACK));
}

auto status::clear() -> void {
  begin();
  printw("\n");
}

auto status::begin() -> void {
  move(get_coord().y, get_coord().x);
}
