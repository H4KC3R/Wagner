#include "WagnerForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
void main() {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);


	Wagner::WagnerForm form;
	Application::Run(% form);
}