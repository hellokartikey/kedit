#include "status.h"

#include <fmt/core.h>

#include "common.h"
#include "file.h"
#include "kedit.h"

auto status::render() -> void {
  // Set Black on White
  attrset(COLOR_PAIR(BLACK_WHITE));

  begin();

  auto l_str = std::string{};
  auto r_str = std::string{};

  // Command
  if ( editor->command_input.size() ) {
    l_str += editor->command_input;
  }

  // Flashed Message
  if ( editor->flash_msg.size() ) {
    l_str += editor->flash_msg;
  }

  // Debug Info
  if ( editor->show_debug_info ) {
    r_str += "DEBUG ";
  }

  // Cursor Position
  const auto& file_obj = editor->file_obj;
  auto edit_curs = editor->file_win.get_curs();
  r_str += fmt::format(
    "[{0}/{2}, {1}] ",
    edit_curs.y + 1,
    edit_curs.x + 1,
    file_obj.size()
    );

  // Mode
  switch ( editor->mode ) {
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

auto status::begin() -> void {
  auto tmp = get_coord();
  move(tmp.y, tmp.x);
}
