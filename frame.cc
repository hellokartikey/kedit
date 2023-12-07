#include "frame.h"

frame::frame() {}

frame::frame(kedit* editor) { set_editor(editor); }

auto frame::set_editor(kedit* editor) -> void { this->editor = editor; }

auto frame::get_editor() -> kedit* { return editor; }
