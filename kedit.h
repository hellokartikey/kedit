#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <curses.h>

#include <fmt/core.h>

#define KEY_ESC 27

enum COLOR {
  BLACK = 0,
  WHITE = 15
};

enum MY_COLOR_PAIR { TEXT, STATUS_BAR };

enum eMode { DEFAULT, COMMAND, INSERT, EXIT };

struct kedit {
  using line = std::string;
  using file = std::vector<line>;

  // Curses Variables
  WINDOW* win;

  int max_x, max_y;
  int size_x, size_y;

  int curs_x = 0;
  int curs_y = 0;

  int cmd_x;

  int ch = 0;

  // Editor Attributes
  enum eMode mode = DEFAULT;
  bool show_debug_info = false;
  bool restricted_cursor = false;

  std::string flash_msg = "";

  // File Handeling
  std::fstream file_stream;
  file file_obj;
  std::string file_name;

  // Commands
  line command_input;

  kedit(int argc, char** argv) {
    // Initialize curses
    win = initscr();

    raw();
    noecho();
    keypad(win, true);
    start_color();

    getmaxyx(win, max_y, max_x);

    size_x = max_x;
    size_y = max_y - 1;

    init_pair(STATUS_BAR, BLACK, WHITE);
    init_pair(TEXT, 15, BLACK);

    // Command line arguements
    if (argc > 2) {
      std::cerr << "Too many arguements" << std::endl;
    } else if (argc == 2) {
      file_name = argv[1];
      open_file();
    } else {
      file_name = "";
      flash("Empty window");
    }

  }

  ~kedit() {
    close_file();
    endwin();
  }

  void open_file() {
    file_stream.open(file_name);

    if (file_stream.fail()) {
      flash("Error openning file");
      return;
    }

    char inp;
    line temp_line;
    while (! file_stream.eof()) {
      file_stream >> std::noskipws >> inp;

      switch (inp) {
      case '\n':
        temp_line.push_back(inp);
        file_obj.push_back(temp_line);
        temp_line.clear();
        break;
      default:
        temp_line.push_back(inp);
      }
    }
    // Last line
    if (temp_line.size()) {
      temp_line.pop_back();
      file_obj.push_back(temp_line);    
    }

    flash(fmt::format("Openned {}", file_name));
  }

  void close_file() {
    file_stream.close();
    file_obj.clear();

    flash("File closed");
  }

  void status_bar() {
    // Clear streams
    std::stringstream status_left;
    std::stringstream status_right;

    // Set status bar color
    attrset(COLOR_PAIR(STATUS_BAR));

    // Print current mode of the editor
    std::string mode_str;
    switch (mode) {
    case DEFAULT:
      mode_str = "DEFAULT";
      break;
    case COMMAND:
      mode_str = "COMMAND";
      break;
    case INSERT:
      mode_str = "INSERT";
      break;
    default:
      break;
    }
    mode_str += " ";

    // Print current command
    // std::string cmd(command_input.begin(), command_input.end());
    std::string cmd = "";
    if (command_input.size()) {
      cmd = std::string(command_input.begin(), command_input.end());
    }

    // Print current cursor coordinates
    std::string coord_fmt = fmt::format("({}, {}) ", curs_x, curs_y);

    // DEBUG
    // Print current input
    std::string debug_inp = "";
    if (show_debug_info) {
      debug_inp = fmt::format("INP: {:x} {} ", ch, ch);
    }

    // Arrange everything into streams
    status_right << debug_inp;
    status_right << coord_fmt;
    status_right << mode_str; 

    status_left << flash_msg;
    status_left << cmd;


    // Pad the remaining bar with white space
    int len_left = status_left.str().size();
    int len_right = status_right.str().size();

    // Move to location of status bar
    move(max_y - 1, 0);

    // Print left side of the status bar
    while (true) {
      std::string str;
      status_left >> str;

      printw("%s ", str.c_str());

      if (status_left.eof()) break;

    }

    // Print middle whitespace
    for (int i = 0; i < max_x - len_left - len_right - 1; i++) {
      printw(" ");
    }

    // Print right side of the status bar
    while (true) {
      std::string str;
      status_right >> str;

      if (status_right.eof()) break;

      printw(" %s", str.c_str());
    }

    // Reset cursor and color
    attrset(COLOR_PAIR(TEXT));
    move_curs();
  }

  void file_window() {
    move(0,0);

    if (! file_stream.is_open()) {
      for (int y = 0; y < size_y; y++) {
        printw("\n");
      }

      return;
    }

    for (auto& l: file_obj) {
      for (auto& ch: l) {
        printw("%c", ch);
      }
    }
    move_curs();
  }

  void default_mode() {
    free_cursor();
    key_move_vim();

    switch (ch) {
    case 'e':
    case 'i':
      mode = INSERT;
      break;
    case ':':
    case 'c':
      mode = COMMAND;
      move(0, size_y);
      break;
    case 'q':
      mode = EXIT;
      break;
    }
  }

  void command_mode() {
    flash();

    if (30 <= ch && ch <= 127) command_input.push_back(char(ch));

    switch (ch) {
    case KEY_BACKSPACE:
      if (command_input.size()) {
        command_input.pop_back();
        cmd_x--;
      }
      break;
    case KEY_ENTER:
    case '\n':
      parse_command();
      move_curs();
      return;
    }

    key_esc_to_default();
  }

  void insert_mode() {
    restrict_cursor();
    key_esc_to_default();
  }

  void run() {

    while (true) {
      if (mode == EXIT) break;
      
      file_window();

      status_bar();

      ch = getch();

      switch (mode) {
      case DEFAULT:
        default_mode();
        break;
      case COMMAND:
        command_mode();
        break;
      case INSERT:
        insert_mode();
        break;
      case EXIT:
        break;
      }

      key_move();

      refresh();
    }
  }

  void key_esc_to_default() {
    switch (ch) {
    case KEY_ESC:
      mode = DEFAULT;
      break;
    }
  }

  void restrict_cursor() {
    restricted_cursor = true;
  }

  void free_cursor() {
    restricted_cursor = false;
  }

  void key_move() {
    switch(ch) {
    case KEY_LEFT:
      dec_curs_x();
      break;
    case KEY_RIGHT:
      inc_curs_x();
      break;
    case KEY_UP:
      dec_curs_y();
      break;
    case KEY_DOWN:
      inc_curs_y();
      break;
    case KEY_HOME:
      home_curs();
      break;
    case KEY_END:
      end_curs();
      break;
    }

    move_curs();
    
    restrict_curs_xy();
  }

  void key_move_vim() {
    switch(ch) {
    case 'h':
      dec_curs_x();
      break;
    case 'l':
      inc_curs_x();
      break;
    case 'k':
      dec_curs_y();
      break;
    case 'j':
      inc_curs_y();
      break;
    }

    move_curs();

    restrict_curs_xy();
  }

  void move_curs() {
    move(curs_y, curs_x);
  }

  void dec_curs_x() {
    if (curs_x == 0 && curs_y == 0) return;

    if (curs_x == 0) {
      curs_x = size_x - 1;
      dec_curs_y();
      return;
    }
    curs_x--;
  }

  void dec_curs_y() {
    if (curs_y == 0) return;

    curs_y--;
  }

  void inc_curs_x() {
    if (curs_x == size_x - 1 && curs_y == size_y - 1) return;

    curs_x++;
    if (curs_x == size_x) {
      curs_x = 0;
      inc_curs_y();
    }
  }

  void inc_curs_y() {
    if (curs_y == size_y - 1) return;
    curs_y++;
  }

  void home_curs() {
    curs_x = 0;
  }

  void end_curs() {
    if (file_obj.size()) {
      curs_x = file_obj[curs_y].size() - 1;
    }
  }

  void restrict_curs_xy() {
    if (! restricted_cursor) return;

    // Y
    if (file_obj.size()) {
      if (curs_y >= file_obj.size()) {
        curs_y = file_obj.size() - 1;
      }
    } else {
      curs_y = 0;
    }

    // X
    if (file_obj.size()) {
      if (curs_x >= file_obj[curs_y].size()) {
        curs_x = file_obj[curs_y].size() - 1;
      }
    } else {
      curs_x = 0;
    }
  }

  void flash(std::string message = "") {
    flash_msg = message;
  }

  void parse_command() {
    std::stringstream cs;
    for (auto& c: command_input) cs << c;

    while (! cs.eof()) {
      std::string s;
      cs >> s;

      // Open file
      if (s == "open") command_open(cs);
      else if (s == "close") command_close(cs);

      // Debug info
      else if (s == "debug") command_debug(cs);

      // Quit 
      else if (s == "quit") command_quit(cs);
      else if (s == "exit") command_quit(cs);
      else if (s == "q") command_quit(cs);

      else {
        flash("Command does not exist");
        mode = DEFAULT;
      }
    }

    // Reset the command stream
    command_input.clear();
    cmd_x = 0;

    move_curs();
  }

  void command_open(std::stringstream& cs) {
    if (cs.eof()) {
      flash("No file name provided");
      return;
    }

    cs >> file_name;
    open_file();

    mode = INSERT;
  }

  void command_close(std::stringstream& cs) {
    close_file();
    mode = DEFAULT;
  }

  void command_debug(std::stringstream& cs) {
    // If no arguements, toggle true/false
    if (cs.eof()) {
      show_debug_info = ! show_debug_info;
      return;
    }

    std::string opt;
    cs >> opt;
    if (opt == "true") show_debug_info = true;
    else if (opt == "false") show_debug_info = false;

    mode = DEFAULT;
  }

  void command_quit(std::stringstream& cs) {
    mode = EXIT;
  }
};
