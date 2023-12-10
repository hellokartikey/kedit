#ifndef KEDIT_KEDIT_H
#define KEDIT_KEDIT_H

#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <curses.h>

#include "common.h"
#include "file.h"
#include "point.h"
#include "status.h"

#define VERSION "0.1"

#define KEY_ESC 27

using namespace std::literals;

class kedit {
 public:
  kedit(int argc, char* argv[]);
  ~kedit();

 private:
  auto color_init() -> void;

  auto file_open() -> void;
  auto file_save() -> void;
  auto file_close() -> void;
  auto file_clear() -> void;

  auto mode_default() -> void;
  auto mode_command() -> void;
  auto mode_insert() -> void;

  auto frame_init() -> void;
  auto frames_resize() -> void;

  auto key_esc_to_default() -> void;
  auto key_move() -> void;
  auto key_move_vim() -> void;

  auto flash(std::string message = ""s) -> void;

  auto command_parse() -> void;
  auto command_default() -> void;
  auto command_open() -> void;
  auto command_close() -> void;
  auto command_debug() -> void;
  auto command_quit() -> void;
  auto command_save() -> void;
  auto command_version() -> void;

 public:
  auto run() -> void;

 private:
  // Curses Variables
  WINDOW* win;

  // Max coordinates of the entire window
  point max;

  // Status Bar
  status status_bar;
  file file_win;

  // Command cursor position
  int cmd_x = 0;

  // Keyboard input
  int ch = 0;

  // Editor Attributes
  modes mode = DEFAULT;
  bool show_debug_info = true;

  std::string flash_msg;

  // File Handeling
  std::fstream file_stream;
  file_vec file_obj;
  std::string file_name;

  // Commands
  line command_input;
  cstream command_stream;

  friend frame;
  friend file;
  friend status;
};

#endif
