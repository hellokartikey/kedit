#include "kedit.h"

kedit::kedit(int argc, char** argv) {
  // Initialize curses
  win = initscr();

  raw();
  noecho();
  keypad(win, true);
  start_color();

  getmaxyx(win, max.y, max.x);
  min = {0, 0};

  size.x = max.x - 2;
  size.y = max.y - 2;

  win_tl.x = 1;
  win_tl.y = 0;

  win_br.x = win_tl.x + size.x - 1;
  win_br.y = win_tl.y + size.y;

  curs.x = win_tl.x;
  curs.y = win_tl.y;

  init_pair(STATUS_BAR, BLACK, WHITE);
  init_pair(TEXT, WHITE, BLACK);

  clear_file_obj();

  // Command line arguements
  if (argc > 2) {
    std::cerr << "Too many arguements" << std::endl;
  } else if (argc == 2) {
    file_name = argv[1];
    file_open();
  } else {
    file_name = "";
    flash("Empty window");
  }

}

kedit::~kedit() {
  file_close();
  endwin();
}

void kedit::file_open() {
  file_stream.open(file_name, std::ios_base::in);

  // If file does not exist
  if (file_stream.fail()) {
    flash("Could not open file, creating new file");
    file_stream.clear();
    return;
  }

  // Clear file_obj
  file_obj.clear();

  char read;
  line current_line = "";
  while (true) {
    file_stream >> std::noskipws >> read;

    if (file_stream.eof()) {
      file_obj.push_back(current_line);
      break;
    };

    current_line.push_back(read);

    if (read == '\n') {
      file_obj.push_back(current_line);
      current_line = "";
    }

    flash(current_line);
  }

  file_stream.close();

  flash(fmt::format("Opened {}", file_name));
}

void kedit::file_save() {
  flash("Saving...");
  file_stream.open(file_name, std::ios_base::out | std::ios_base::trunc);

  if (file_stream.fail()) {
    file_stream.clear();
    file_stream.close();
    flash("Error saving file");
    return;
  }

  for (auto& line: file_obj) {
    file_stream << line;
  }


  file_stream.close();

  flash("Saved");
}

void kedit::file_close() {
  file_stream.close();
  clear_file_obj();

  flash("File closed");
}

void kedit::clear_file_obj() {
  file_obj = { "" };
}

void kedit::status_bar() {
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
  std::string coord_fmt = fmt::format("({}, {}) ", curs.x, curs.y);

  // DEBUG
  // Print current input
  std::string debug_inp = "";
  if (show_debug_info) {
    point index = {curs.x - win_tl.x, curs.y - win_tl.y};
    debug_inp = fmt::format("({}, {}) ", index.x, index.y);

    std::string s = "";
    s.insert(0, 1, 'c');
    debug_inp += fmt::format("s = {} ", s);
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
  move(max.y - 1, 0);

  // Print left side of the status bar
  while (true) {
    std::string str;
    status_left >> str;

    printw("%s ", str.c_str());

    if (status_left.eof()) break;

  }

  // Print middle whitespace
  for (int i = 0; i < max.x - len_left - len_right - 1; i++) {
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
  curs_move();
}

void kedit::scroll_bar() {
  point bar {win_br.x + 1, win_tl.y};
  point bar_end {win_br.x + 1, win_br.y};

  for (; bar.y <= bar_end.y; bar.y++) mvaddch(bar.y, bar.x , '.');

  curs_move();

}

void kedit::file_window() {
  curs = win_tl;

  for (auto& line_str: file_obj) {
    for (auto& _: line_str) {
      mvaddch(curs.y, curs.x, _);
      curs_inc_x();
    }
  }

  int y = getcury(win);
  while (y++ <= win_br.y) {
    printw("\n");
  }

  curs_move();
}

void kedit::mode_default() {
  key_move_vim();

  switch (ch) {
  case 'e':
  case 'i':
    mode = INSERT;
    break;
  case ':':
  case 'c':
    mode = COMMAND;
    move(0, win_br.y);
    break;
  case 'q':
    mode = EXIT;
    break;
  }
}

void kedit::mode_command() {
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
    command_parse();
    curs_move();
    return;
  }

  key_esc_to_default();
}

void kedit::mode_insert() {

  if (0x20 <= ch && ch <= 0x7f) {
    insert_char();
  }

  switch (ch) {
  case KEY_BACKSPACE:
    insert_backspace();
    break;
  case KEY_DC:
    insert_delete();
    break;
  case KEY_ENTER:
  case '\n':
    insert_line();
    break;
  }

  key_esc_to_default();
}

void kedit::insert_char() {
  point index = {curs.x - win_tl.x, curs.y - win_tl.y};
  file_obj[index.y] += char(ch);
  curs_inc_x();
}

void kedit::insert_backspace() {
  if (curs.x == 0) {
    if (curs.y) {
      curs_dec_y();
      curs_end();
      file_obj[curs.y].pop_back();
      file_obj[curs.y] += file_obj[curs.y + 1];
      file_obj.erase(file_obj.begin() + curs.y + 1);
    }

    return;
  }
  file_obj[curs.y].erase(curs.x, 1);
  curs_dec_x();
}

void kedit::insert_delete() {
  if (curs.x == file_obj[curs.y].size() - 1) {
    if (curs.y == file_obj.size() - 1) return;
    file_obj[curs.y].pop_back();

    if (file_obj[curs.y].size() == 0) {
      file_obj.erase(file_obj.begin() + curs.y);
    }

    if (file_obj.size() == 0) {
      line temp = "\n";
      file_obj.push_back(temp);
    }

    return;
  }

  file_obj[curs.y].erase(curs.x, 1);
}

void kedit::insert_line() {
  line temp_line_break = file_obj[curs.y].substr(0, curs.x) + '\n';
  line temp_line_new = file_obj[curs.y].substr(curs.x, file_obj[curs.y].size());

  file_obj.insert(file_obj.begin() + curs.y + 1, temp_line_new);
  file_obj[curs.y] = temp_line_break;

  curs_inc_y();
  curs_home();
}

void kedit::run() {

  while (true) {
    if (mode == EXIT) break;

    file_window();

    status_bar();

    scroll_bar();

    curs_restrict_yx();

    ch = getch();

    switch (mode) {
    case DEFAULT:
      mode_default();
      break;
    case COMMAND:
      mode_command();
      break;
    case INSERT:
      mode_insert();
      break;
    case EXIT:
      break;
    }

    key_move();

    refresh();
  }
}

void kedit::key_esc_to_default() {
  switch (ch) {
  case KEY_ESC:
    mode = DEFAULT;
    break;
  }
}

void kedit::key_move() {
  switch(ch) {
  case KEY_LEFT:
    curs_dec_x();
    break;
  case KEY_RIGHT:
    curs_inc_x();
    break;
  case KEY_UP:
    curs_dec_y();
    break;
  case KEY_DOWN:
    curs_inc_y();
    break;
  case KEY_HOME:
    curs_home();
    break;
  case KEY_END:
    curs_end();
    break;
  }

  curs_move();
  
  curs_restrict_yx();
}

void kedit::key_move_vim() {
  switch(ch) {
  case 'h':
    curs_dec_x();
    break;
  case 'l':
    curs_inc_x();
    break;
  case 'k':
    curs_dec_y();
    break;
  case 'j':
    curs_inc_y();
    break;
  }

  curs_move();

  curs_restrict_yx();
}

void kedit::curs_move() {
  move(curs.y, curs.x);
}

void kedit::curs_dec_x() {
  if (curs.x == win_tl.x && curs.y == win_tl.y) return;

  if (curs.x == win_tl.x) {
    curs.x = win_br.x;
    curs_dec_y();
    return;
  }
  curs.x--;
}

void kedit::curs_dec_y() {
  if (curs.y == win_tl.y) return;

  curs.y--;
}

void kedit::curs_inc_x() {
  if (curs.x == win_br.x && curs.y == win_br.y) return;

  curs.x++;
  if (curs.x > win_br.x) {
    curs.x = win_tl.x;
    curs_inc_y();
  }
}

void kedit::curs_inc_y() {
  if (curs.y == win_br.y) return;
  curs.y++;
}

void kedit::curs_home() {
  curs.x = win_tl.x;
}

void kedit::curs_end() {
  if (file_obj[curs.y].size()) {
    curs.x = file_obj[curs.y].size() - 1;
  } else {
    curs.x = win_tl.x;
  }
}

void kedit::curs_restrict_yx() {
  if (mode != INSERT) return;

  // Y
  if (file_obj.size()) {
    if (curs.y >= file_obj.size()) {
      curs.y = file_obj.size() - 1;
    }
  } else {
    curs.y = 0;
  }

  // X
  if (int line_size = file_obj[curs.y].size()) {
    if (line_size == 0) {
      curs.x = win_tl.x;
    }
    if (curs.x >= line_size) {
      curs.x = line_size - 1;
    }
  } else {
    curs.x = 0;
  }
}

void kedit::flash(std::string message) {
  flash_msg = message;
}

void kedit::command_parse() {
  std::stringstream cs;
  for (auto& c: command_input) cs << c;

  while (! cs.eof()) {
    std::string s;
    cs >> s;

    // Open file
    if (s == "open") command_open(cs);
    else if (s == "o") command_open(cs);

    else if (s == "close") command_close(cs);
    else if (s == "c") command_close(cs);

    // Save file
    else if (s == "save") command_save(cs);
    else if (s == "s") command_save(cs);

    // Debug info
    else if (s == "debug") command_debug(cs);

    // Version info
    else if (s == "version") command_version(cs);
    else if (s == "v") command_version(cs);

    // Quit 
    else if (s == "quit") command_quit(cs);
    else if (s == "exit") command_quit(cs);
    else if (s == "q") command_quit(cs);

    else command_default(cs);
  }

  // Reset the command stream
  command_input.clear();
  cmd_x = 0;

  curs_move();
}

void kedit::command_default(std::stringstream& cs) {
  flash("Command does not exist");
  mode = DEFAULT;
}

void kedit::command_open(std::stringstream& cs) {
  if (cs.eof()) {
    flash("No file name provided");
    return;
  }

  cs >> file_name;
  file_open();

  mode = INSERT;
}

void kedit::command_close(std::stringstream& cs) {
  file_close();
  mode = DEFAULT;
}

void kedit::command_debug(std::stringstream& cs) {
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

void kedit::command_quit(std::stringstream& cs) {
  mode = EXIT;
}

void kedit::command_save(std::stringstream& cs) {
  if (! cs.eof()) {
    cs >> file_name;
  }
  
  file_save();
  mode = DEFAULT;
}

void kedit::command_version(std::stringstream& cs) {
  flash(fmt::format("{}", VERSION));
  mode = DEFAULT;
}
