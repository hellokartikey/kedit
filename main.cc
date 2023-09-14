#include "kedit.h"

int main(int argc, char** argv) {
    kedit editor(argc, argv);

    editor.run();

    return 0;
}