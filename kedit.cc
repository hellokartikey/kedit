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

  frame_init();

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

auto kedit::mode_default() -> void {
  key_move_vim();

  switch ( ch ) {
  case 'e':
  case 'i':
    mode = INSERT;
    break;
  case ':':
  case 'c':
    mode = COMMAND;
    break;
  case 'q':
    mode = EXIT;
    break;
  }
}

auto kedit::mode_command() -> void {
  flash();

  if ( 30 <= ch && ch <= 127 ) command_input.push_back(char(ch));

  switch ( ch ) {
  case KEY_BACKSPACE:
    if ( command_input.size()) {
      command_input.pop_back();
      cmd_x--;
    }
    break;
  case KEY_ENTER:
  case '\n':
    command_parse();
    return;
  }

  key_esc_to_default();
}

auto kedit::mode_insert() -> void {
  if ( 0x20 <= ch && ch <= 0x7e ) {
    file_win.insert_char(ch);
  }

  switch ( ch ) {
  case KEY_BACKSPACE:
    file_win.insert_backspace();
    break;
  case KEY_DC:
    file_win.insert_delete();
    break;
  case KEY_ENTER:
  case '\n':
    file_win.insert_line();
    break;
  }

  key_esc_to_default();
}

auto kedit::frame_init() -> void {
  status_bar.init(this);
  file_win.init(this);
}

auto kedit::frames_resize() -> void {
  status_bar.resize({0, max.y - 1}, {max.x, max.y - 1});
  file_win.resize({1, 0}, {max.x - 1, max.y - 2});
}

auto kedit::run() -> void {
  frames_resize();

  while ( true ) {
    if ( mode == EXIT ) break;

    clear();

    file_win.render();
    status_bar.render();
    file_win.curs_move();

    ch = getch();

    switch ( mode ) {
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
  switch ( ch ) {
  case KEY_ESC:
    mode = DEFAULT;
    break;
  }
}

auto kedit::key_move() -> void {
  switch( ch ) {
  case KEY_LEFT:
    file_win.curs_dec_x();
    break;
  case KEY_RIGHT:
    file_win.curs_inc_x();
    break;
  case KEY_UP:
    file_win.curs_dec_y();
    break;
  case KEY_DOWN:
    file_win.curs_inc_y();
    break;
  case KEY_HOME:
    file_win.curs_home();
    break;
  case KEY_END:
    file_win.curs_end();
    break;
  }
}

auto kedit::key_move_vim() -> void {
  switch( ch ) {
  case 'h':
    file_win.curs_dec_x();
    break;
  case 'l':
    file_win.curs_inc_x();
    break;
  case 'k':
    file_win.curs_dec_y();
    break;
  case 'j':
    file_win.curs_inc_y();
    break;
  }
}

auto kedit::flash(std::string message) -> void {
  flash_msg = message;
}

auto kedit::command_parse() -> void {
  command_stream.str(command_input);

  while ( not command_stream.eof() ) {
    std::string s;
    command_stream >> s;

    // Open file
    if ( s == "open" ) command_open();
    else if ( s == "o" ) command_open();

    else if ( s == "close" ) command_close();
    else if ( s == "c" ) command_close();

    // Save file
    else if ( s == "save" ) command_save();
    else if ( s == "s" ) command_save();

    // Debug info
    else if ( s == "debug" ) command_debug();

    // Version info
    else if ( s == "version" ) command_version();
    else if ( s == "v" ) command_version();

    // Quit 
    else if ( s == "quit" ) command_quit();
    else if ( s == "exit" ) command_quit();
    else if ( s == "q" ) command_quit();

    else command_default();
  }

  // Reset the command stream
  command_input.clear();
  command_stream.str(""s);
  command_stream.clear();
  cmd_x = 0;
}

auto kedit::command_default() -> void {
  flash("Command does not exist");
  mode = DEFAULT;
}

auto kedit::command_open() -> void {
  if ( command_stream.eof() ) {
    flash("No file name provided");
    return;
  }

  command_stream >> file_name;
  file_open();

  mode = INSERT;
}

auto kedit::command_close() -> void {
  file_close();
  file_clear();
  mode = DEFAULT;
}

auto kedit::command_debug() -> void {
  // If no arguements, toggle true/false
  if ( command_stream.eof() ) {
    show_debug_info = ! show_debug_info;
  } else {
    std::string opt;
    command_stream >> opt;
    if ( opt == "true" ) show_debug_info = true;
    else if ( opt == "false" ) show_debug_info = false;
  }

  mode = DEFAULT;
}

auto kedit::command_quit() -> void {
  mode = EXIT;
}

auto kedit::command_save() -> void {
  if ( not command_stream.eof() ) {
    command_stream >> file_name;
  }
  
  file_save();
  mode = DEFAULT;
}

auto kedit::command_version() -> void {
  flash(fmt::format("{}", VERSION));
  mode = DEFAULT;
}

auto kedit::get_command() -> const std::string& { return command_input; }

auto kedit::get_flash() -> const std::string& { return flash_msg; }

auto kedit::get_mode() -> enum modes { return mode; }

auto kedit::get_file() -> file_vec& { return file_obj; }

auto kedit::get_debug_status() -> bool { return show_debug_info; }

auto kedit::get_size() -> const point& { return max; }

auto kedit::get_file_frame() -> const file& { return file_win; }

auto kedit::get_status_bar() -> const status& { return status_bar; }
