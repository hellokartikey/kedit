#include "kedit.h"
#include "common.h"
#include "status.h"

#include <curses.h>
#include <fmt/core.h>

using namespace std::literals;

kedit::kedit(int argc, char* argv[]) {
  // Initialize curses
  win = initscr();

  raw();
  noecho();
  keypad(win, true);

  getmaxyx(win, max.y, max.x);

  min = {0, 0};
  size = max - point{2, 2};
  win_tl = {1, 0};
  win_br = win_tl + size - point{1, 0};
  curs = win_tl;

  status_bar.init(this, {0, max.y - 1}, {max.x, max.y - 1});

  color_init();

  file_clear();

  // Command line arguements
  if ( argc > 2 ) {
    fmt::print(stderr, "Too many arguements\n");
  } else if ( argc == 2 ) {
    file_name = argv[1];
    file_open();
  } else {
    file_name = ""s;
    flash("Empty window");
  }

}

kedit::~kedit() {
  file_close();
  endwin();
}

auto kedit::color_init() -> void {
  if ( not has_colors() ) {
    fmt::print(stderr, "This terminal does not support colors\n");
    return;
  }

  if ( not can_change_color() ) {
    fmt::print(stderr, "This terminal does not support custom colors\n");
    return;
  }

  start_color();

  init_color(COLOR_BLACK, 0, 0, 0);
  init_color(COLOR_WHITE, 1000, 1000, 1000);

  init_pair(BLACK_WHITE, COLOR_BLACK, COLOR_WHITE);
  init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
}

auto kedit::file_open() -> void {
  file_stream.open(file_name, std::ios_base::in);

  // If file does not exist
  if ( file_stream.fail() ) {
    flash("Could not open file, creating new file");
    file_stream.clear();
    return;
  }

  // Clear file_obj
  file_obj.clear();

  auto readline = line{};
  while ( std::getline(file_stream, readline) ) {
    file_obj.push_back(readline);
  }

  file_stream.close();

  flash(fmt::format("Opened {}", file_name));
}

auto kedit::file_save() -> void {
  flash("Saving...");
  file_stream.open(file_name, std::ios_base::out | std::ios_base::trunc);

  if ( file_stream.fail() ) {
    file_stream.clear();
    file_stream.close();
    flash("Error saving file");
    return;
  }

  // Inserts a new empty line at end of the file if not exist
  for ( const auto& line: file_obj ) {
    file_stream << line << "\n";
  }

  file_stream.close();

  flash("Saved");
}

auto kedit::file_close() -> void {
  file_stream.close();
  file_clear();

  flash("File closed");
}

auto kedit::file_clear() -> void {
  file_obj.clear();
}

auto kedit::scroll_bar() -> void {
  point bar {win_br.x + 1, win_tl.y};
  point bar_end {win_br.x + 1, win_br.y};

  for (; bar.y <= bar_end.y; bar.y++) mvaddch(bar.y, bar.x , '.');

  curs_move();

}

auto kedit::file_window() -> void {
  curs = win_tl;

  for ( const auto& line_str: file_obj ) {
    for ( const auto& _: line_str ) {
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

auto kedit::mode_default() -> void {
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

auto kedit::mode_command() -> void {
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

auto kedit::mode_insert() -> void {

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

auto kedit::insert_char() -> void {
  point index = {curs.x - win_tl.x, curs.y - win_tl.y};
  file_obj[index.y] += char(ch);
  curs_inc_x();
}

auto kedit::insert_backspace() -> void {
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

auto kedit::insert_delete() -> void {
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

auto kedit::insert_line() -> void {
  line temp_line_break = file_obj[curs.y].substr(0, curs.x) + '\n';
  line temp_line_new = file_obj[curs.y].substr(curs.x, file_obj[curs.y].size());

  file_obj.insert(file_obj.begin() + curs.y + 1, temp_line_new);
  file_obj[curs.y] = temp_line_break;

  curs_inc_y();
  curs_home();
}

auto kedit::run() -> void {

  while (true) {
    if (mode == EXIT) break;

    file_window();

    status_bar.render();

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

auto kedit::key_esc_to_default() -> void {
  switch (ch) {
  case KEY_ESC:
    mode = DEFAULT;
    break;
  }
}

auto kedit::key_move() -> void {
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

auto kedit::key_move_vim() -> void {
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

auto kedit::curs_move() -> void {
  move(curs.y, curs.x);
}

auto kedit::curs_dec_x() -> void {
  if (curs.x == win_tl.x && curs.y == win_tl.y) return;

  if (curs.x == win_tl.x) {
    curs.x = win_br.x;
    curs_dec_y();
    return;
  }
  curs.x--;
}

auto kedit::curs_dec_y() -> void {
  if (curs.y == win_tl.y) return;

  curs.y--;
}

auto kedit::curs_inc_x() -> void {
  if (curs.x == win_br.x && curs.y == win_br.y) return;

  curs.x++;
  if (curs.x > win_br.x) {
    curs.x = win_tl.x;
    curs_inc_y();
  }
}

auto kedit::curs_inc_y() -> void {
  if (curs.y == win_br.y) return;
  curs.y++;
}

auto kedit::curs_home() -> void {
  curs.x = win_tl.x;
}

auto kedit::curs_end() -> void {
  if (file_obj[curs.y].size()) {
    curs.x = file_obj[curs.y].size() - 1;
  } else {
    curs.x = win_tl.x;
  }
}

auto kedit::curs_restrict_yx() -> void {
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

auto kedit::flash(std::string message) -> void {
  flash_msg = message;
}

auto kedit::command_parse() -> void {
  cstream cs;
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

auto kedit::command_default(cstream& cs) -> void {
  flash("Command does not exist");
  mode = DEFAULT;
}

auto kedit::command_open(cstream& cs) -> void {
  if (cs.eof()) {
    flash("No file name provided");
    return;
  }

  cs >> file_name;
  file_open();

  mode = INSERT;
}

auto kedit::command_close(cstream& cs) -> void {
  file_close();
  mode = DEFAULT;
}

auto kedit::command_debug(cstream& cs) -> void {
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

auto kedit::command_quit(cstream& cs) -> void {
  mode = EXIT;
}

auto kedit::command_save(cstream& cs) -> void {
  if (! cs.eof()) {
    cs >> file_name;
  }
  
  file_save();
  mode = DEFAULT;
}

auto kedit::command_version(cstream& cs) -> void {
  flash(fmt::format("{}", VERSION));
  mode = DEFAULT;
}

auto kedit::get_command() -> std::string const { return command_input; }

auto kedit::get_flash() -> std::string const { return flash_msg; }

auto kedit::get_curs() -> point const { return curs; }

auto kedit::get_mode() -> enum modes const { return mode; }
