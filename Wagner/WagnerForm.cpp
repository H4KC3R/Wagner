#include "WagnerForm.h"
#include <string>

void Wagner::WagnerForm::OnDragDrop(System::Object^ sender, System::Windows::Forms::DragEventArgs^ e) {
    array<String^>^ paths = safe_cast<array<String^>^>(e->Data->GetData(DataFormats::FileDrop));
    for each (String ^ path in paths) {
        String^ ext = System::IO::Path::GetExtension(path)->ToLower();
        if (ext == ".txt") CyclogrammTextBox->AppendText(System::IO::File::ReadAllText(path));
    }
}

