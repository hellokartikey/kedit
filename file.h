#ifndef KEDIT_FILE_H
#define KEDIT_FILE_H

#include "frame.h"

class file : public frame {
 public:
  auto render() -> void override;

  auto insert_char(char ch) -> void;
  auto insert_backspace() -> void;
  auto insert_delete() -> void;
  auto insert_line() -> void;

 private:
  auto begin() -> void;
};

#endif
