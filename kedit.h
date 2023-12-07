#ifndef KEDIT_KEDIT_H
#define KEDIT_KEDIT_H

#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <curses.h>

#include "common.h"
#include "point.h"
#include "status.h"

#define VERSION "0.1"

#define KEY_ESC 27

using namespace std::literals;

class kedit {
 public:
  kedit(int argc, char* argv[]);
  ~kedit();

  auto color_init() -> void;

  auto file_open() -> void;
  auto file_save() -> void;
  auto file_close() -> void;
  auto file_clear() -> void;

  // auto status_bar() -> void;
  auto scroll_bar() -> void;
  auto file_window() -> void;

  auto mode_default() -> void;
  auto mode_command() -> void;
  auto mode_insert() -> void;

  auto insert_char() -> void;
  auto insert_backspace() -> void;
  auto insert_delete() -> void;
  auto insert_line() -> void;

  auto run() -> void;

  auto key_esc_to_default() -> void;
  auto key_move() -> void;
  auto key_move_vim() -> void;

  auto curs_move() -> void;
  auto curs_dec_x() -> void;
  auto curs_dec_y() -> void;
  auto curs_inc_x() -> void;
  auto curs_inc_y() -> void;
  auto curs_home() -> void;
  auto curs_end() -> void;
  auto curs_restrict_yx() -> void;

  auto flash(std::string message = ""s) -> void;

  auto command_parse() -> void;
  auto command_default(cstream& cs) -> void;
  auto command_open(cstream& cs) -> void;
  auto command_close(cstream& cs) -> void;
  auto command_debug(cstream& cs) -> void;
  auto command_quit(cstream& cs) -> void;
  auto command_save(cstream& cs) -> void;
  auto command_version(cstream& cs) -> void;

 public:
  auto get_command() -> std::string const;
  auto get_flash() -> std::string const;
  auto get_curs() -> point const;
  auto get_mode() -> enum modes const;

 private:
  // Curses Variables
  WINDOW* win;

  // Max coordinates of the entire window
  point max;
  point min;

  // Max coordinates of the text window
  point size;

  // Coordinates of the text window
  point win_tl, win_br;

  // Cursor coordinates
  point curs;

  // Status Bar
  status status_bar;

  // Command cursor position
  int cmd_x;

  // Keyboard input
  int ch = 0;

  // Editor Attributes
  modes mode = DEFAULT;
  bool show_debug_info = true;
  bool restricted_cursor = false;

  std::string flash_msg = "";

  // File Handeling
  std::fstream file_stream;
  file file_obj;
  std::string file_name;

  // Commands
  line command_input;
};

#endif
