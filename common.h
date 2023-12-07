#ifndef KEDIT_COMMON_H
#define KEDIT_COMMON_H

#include <string>
#include <vector>

using line = std::string;

using file_vec = std::vector<line>;

using cstream = std::stringstream;

enum MY_COLOR_PAIR { WHITE_BLACK, BLACK_WHITE };

enum modes { DEFAULT, COMMAND, INSERT, EXIT };

#endif
