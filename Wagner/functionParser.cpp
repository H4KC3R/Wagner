#include "functionParser.h"


void getFuncName(System::String^ s) {
    int charLocation = s->IndexOf("(", System::StringComparison::Ordinal);
    if (charLocation > 0)
    {
        System::Console::WriteLine(s->Substring(0, charLocation));
    }
}

void getFuncArgs();