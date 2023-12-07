#include "file.h"

#include "kedit.h"

auto file::render() -> void {
  attrset(COLOR_PAIR(WHITE_BLACK));

  auto pos = curs;

  begin();

  const auto& file_obj = editor->get_file();
  for ( const auto& l: file_obj ) {
    curs_home();

    curs_move();

    for ( const auto& ch: l) {
      addch(ch);
      curs_inc_x();
    }

    curs_inc_y();
  }

  attrset(COLOR_PAIR(WHITE_BLACK));

  curs = pos;

  curs_move();

  curs_restrict();
}

auto file::insert_char(char ch) -> void {
  auto& file_obj = editor->get_file();

  if ( curs.y < file_obj.size() ) {
    file_obj[curs.y].insert(curs.x, 1, ch);
  } else {
    file_obj.push_back( std::string{ char(ch) } );
  }

  curs_inc_x();
}

auto file::insert_backspace() -> void {
  auto& file_obj = editor->get_file();

  if ( curs.x ) {
    file_obj[curs.y].erase(curs.x - 1, 1);
    curs_dec_x();
    return;
  }

  if ( curs.x == 0 and curs.y ) {
    auto idx = curs;

    curs_dec_y();
    curs_end();

    file_obj[idx.y - 1] += file_obj[idx.y];
    file_obj.erase(file_obj.begin() + idx.y);
  }
}

auto file::insert_delete() -> void {
  auto& file_obj = editor->get_file();
  auto max = editor->get_size();

  // TODO
}

auto file::insert_line() -> void {
  auto& file_obj = editor->get_file();

  auto idx = curs;

  curs_inc_y();
  curs_home();

  auto new_line = file_obj[idx.y].substr(idx.x);
  auto prev_line = file_obj[idx.y].substr(0, idx.x);

  file_obj[idx.y] = prev_line;
  file_obj.insert(file_obj.begin() + idx.y + 1, new_line);
}

auto file::begin() -> void {
  curs = {0, 0};
}
