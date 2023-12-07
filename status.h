#ifndef KEDIT_STATUS_H
#define KEDIT_STATUS_H

#include "frame.h"

class status : public frame {
 public:
  auto render() -> void override;

 private:
  auto begin() -> void;
};

#endif
