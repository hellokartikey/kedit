#include "kedit.h"

auto main(int argc, char* argv[]) -> int {
    auto editor = kedit{argc, argv};

    editor.run();

    return 0;
}