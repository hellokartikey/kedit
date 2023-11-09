#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <curses.h>

#define VERSION "0.1"

#define KEY_ESC 27

struct kedit {
  using line = std::string;

  using file = std::vector<line>;

  using cstream = std::stringstream;

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

  kedit(int argc, char* argv[]);
  ~kedit();

  auto file_open() -> void;
  auto file_save() -> void;
  auto file_close() -> void;
  auto clear_file_obj() -> void;

  auto status_bar() -> void;
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

  auto flash(std::string message = "") -> void;

  auto command_parse() -> void;
  auto command_default(cstream& cs) -> void;
  auto command_open(cstream& cs) -> void;
  auto command_close(cstream& cs) -> void;
  auto command_debug(cstream& cs) -> void;
  auto command_quit(cstream& cs) -> void;
  auto command_save(cstream& cs) -> void;
  auto command_version(cstream& cs) -> void;
};
