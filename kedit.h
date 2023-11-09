#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <curses.h>

#include <fmt/core.h>

#define VERSION "0.1"

#define KEY_ESC 27

struct kedit {
  using line = std::string;

  using file = std::vector<line>;

  enum COLOR { BLACK = 0, WHITE = 15 };

  enum MY_COLOR_PAIR { TEXT, STATUS_BAR };

  enum modes { DEFAULT, COMMAND, INSERT, EXIT };

  struct point { int x = 0, y = 0; };

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

  kedit(int argc, char** argv);
  ~kedit();

  void file_open();
  void file_save();
  void file_close();
  void clear_file_obj();

  void status_bar();
  void scroll_bar();
  void file_window();

  void mode_default();
  void mode_command();
  void mode_insert();

  void insert_char();
  void insert_backspace();
  void insert_delete();
  void insert_line();

  void run();

  void key_esc_to_default();
  void key_move();
  void key_move_vim();

  void curs_move();
  void curs_dec_x();
  void curs_dec_y();
  void curs_inc_x();
  void curs_inc_y();
  void curs_home();
  void curs_end();
  void curs_restrict_yx();

  void flash(std::string message = "");

  void command_parse();
  void command_default(std::stringstream& cs);
  void command_open(std::stringstream& cs);
  void command_close(std::stringstream& cs);
  void command_debug(std::stringstream& cs);
  void command_quit(std::stringstream& cs);
  void command_save(std::stringstream& cs);
  void command_version(std::stringstream& cs);
};
