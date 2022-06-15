#include "WagnerForm.h"

System::Void Wagner::WagnerForm::ExpandButton_Click(System::Object^ sender, System::EventArgs^ e) {
	if (ExpandButton->Text == ">") {
		this->Width = 720;
		ExpandButton->Text = "<";
	}
	else {
		this->Width = 575;
		ExpandButton->Text = ">";
	}
}

System::Void Wagner::WagnerForm::StartButton_Click(System::Object^ sender, System::EventArgs^ e) {
	if (CyclogrammTextBox->Text->Length == 0)
		return;
	if (!isScriptValid)
		return;
	Console::WriteLine("i am here");
}

System::Void Wagner::WagnerForm::WagnerForm_Load(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->AllowDrop = true;
	CyclogrammTextBox->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &Wagner::WagnerForm::OnDragDrop);
	CyclogrammTextBox->Clear();

	server = gcnew SimpleTcpServer(ServerAdrress->Text);
	server->Events->ClientConnected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnClientConnected);
	server->Events->ClientDisconnected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnClientDisconnected);
	server->Events->DataReceived += gcnew System::EventHandler<SuperSimpleTcp::DataReceivedEventArgs^>(this, &Wagner::WagnerForm::OnDataReceived);
	server->Start();
	chatTextBox->Text += "Запуск....\r\n";

	funcDictionary->Add("getPosition", 0);
	funcDictionary->Add("moveTo", 1);
	funcDictionary->Add("park", 2);
	funcDictionary->Add("getErrors", 3);
	funcDictionary->Add("resetErrors", 4);
}

System::Void Wagner::WagnerForm::CommandListBox_MouseDoubleClick(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
	CyclogrammTextBox->Text += CommandListBox->SelectedItem->ToString() + "()" + "\r\n";
	ValidateText();
}

void Wagner::WagnerForm::OnDragDrop(System::Object^ sender, System::Windows::Forms::DragEventArgs^ e) {
	array<String^>^ paths = safe_cast<array<String^>^>(e->Data->GetData(DataFormats::FileDrop));
	for each (String ^ path in paths) {
		String^ ext = System::IO::Path::GetExtension(path)->ToLower();
		if (ext == ".txt") CyclogrammTextBox->AppendText(System::IO::File::ReadAllText(path));
	}
	ValidateText();
}

System::Void Wagner::WagnerForm::ClearCyclogrammButton_Click(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Clear();
}

#pragma region Server

void Wagner::WagnerForm::UpdateChatBox(String^ text) {
	chatTextBox->Text += text;
}

void Wagner::WagnerForm::UpdateClientConnected(String^ ip) {
	chatTextBox->Text += String::Format(gcnew String("{0}: подключен\r\n"), ip);
	clientsListBox->Items->Add(ip);

}

void Wagner::WagnerForm::UpdateClientDisconnected(String^ ip) {
	chatTextBox->Text += String::Format(gcnew String("{0}: отключен\r\n"), ip);
	clientsListBox->Items->Remove(ip);
}

void Wagner::WagnerForm::OnClientConnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	Update^ action = gcnew Wagner::WagnerForm::Update(this, &Wagner::WagnerForm::UpdateClientConnected);
	this->Invoke(action, e->IpPort);
}

void Wagner::WagnerForm::OnClientDisconnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateClientDisconnected);
	this->Invoke(action, e->IpPort);
}

void Wagner::WagnerForm::OnDataReceived(System::Object^ sender, SuperSimpleTcp::DataReceivedEventArgs^ e) {
	return;
}

#pragma endregion

#pragma region MarshallingPackets

array<System::Byte>^ Wagner::WagnerForm::getBytes(WagnerPacket^ packet) {
	int size = Marshal::SizeOf(packet);
	array<Byte>^ arr = gcnew array<Byte>(size);

	IntPtr ptr = Marshal::AllocHGlobal(size);
	Marshal::StructureToPtr(packet, ptr, true);
	Marshal::Copy(ptr, arr, 0, size);
	Marshal::FreeHGlobal(ptr);
	return arr;
}

Wagner::WagnerForm::WagnerPacket^ Wagner::WagnerForm::fromBytes(array<Byte>^ arr) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	int size = Marshal::SizeOf(packet);
	IntPtr ptr = Marshal::AllocHGlobal(size);

	Marshal::Copy(arr, 0, ptr, size);

	packet = (WagnerPacket^)Marshal::PtrToStructure(ptr, packet->GetType());
	Marshal::FreeHGlobal(ptr);

	return packet;
}

#pragma endregion

#pragma region ValidateTextBox

System::String^ Wagner::WagnerForm::getFunctionFromString(String^ s) {
	int charLocation = s->IndexOf("(", StringComparison::Ordinal);
	if (charLocation > 0) {
		return s->Substring(0, charLocation);
	}
	return String::Empty;
}

System::Collections::Generic::List<uint32_t>^ Wagner::WagnerForm::getArgsFromString(String^ s) {
	List<uint32_t>^ args = gcnew List<uint32_t>();
	int Pos1 = s->IndexOf("(") + 1;
	int Pos2 = s->IndexOf(")");
	if ((Pos2 - Pos1) <= 0) {
		return args;
	}
	auto args_string = (s->Substring(Pos1, Pos2 - Pos1))->Split(',');
	for each (String ^ temp in args_string) {
		uint32_t num;
		if (System::UInt32::TryParse(temp, num))
			args->Add(num);
	}
	return args;
}

bool Wagner::WagnerForm::functionParser(String^ s) {
	bool isCorrectName = false;
	bool isCorrectArgs = false;
	int startOfArgs = s->IndexOf("(") + 1;
	int endOfArgs = s->IndexOf(")");
	if (s->Length - endOfArgs != 1)
		return false;

	String^ funcName = getFunctionFromString(s);
	if (String::IsNullOrEmpty(funcName))
		return false;	
	
	if (funcDictionary->ContainsKey(funcName))
		isCorrectName = true;
	
	auto args = getArgsFromString(s);
	if (funcName == "moveTo" && args->Count == 1)
		isCorrectArgs = true;
	else if (isCorrectName && funcName != "moveTo" && (endOfArgs - startOfArgs) == 0)
		isCorrectArgs = true;
	return (isCorrectName && isCorrectArgs);
}

void Wagner::WagnerForm::ValidateText() {
	System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::WaitCursor;
	auto textLines = CyclogrammTextBox->Text->Split('\n');
	isScriptValid = true;
	for (int i = 0; i < textLines->Length; i++) {
		if (String::IsNullOrEmpty(textLines[i]))
			continue;
		int start = CyclogrammTextBox->GetFirstCharIndexFromLine(i);
		int length = CyclogrammTextBox->Lines[i]->Length;
		CyclogrammTextBox->Select(start, length);
		if (!functionParser(textLines[i])) {
			CyclogrammTextBox->SelectionFont = gcnew System::Drawing::Font(CyclogrammTextBox->SelectionFont, FontStyle::Underline);
			CyclogrammTextBox->SelectionColor = Color::DarkBlue;
			isScriptValid = false;
		}
		else {
			CyclogrammTextBox->SelectionFont = gcnew System::Drawing::Font(CyclogrammTextBox->SelectionFont, FontStyle::Regular);
			CyclogrammTextBox->SelectionColor = Color::Black;
		}
	}
	System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::Default;
}

System::Void Wagner::WagnerForm::CyclogrammTextBox_Leave(System::Object^ sender, System::EventArgs^ e) {
	ValidateText();
}

#pragma endregion

void Wagner::WagnerForm::doFunction(uint8_t func, uint32_t dataToSend) {
	WagnerPacket^ packet = gcnew WagnerPacket();
	packet->command = func;
	packet->data = dataToSend;
	
	auto message = getBytes(packet);
	server->Send(clientsListBox->SelectedItem->ToString(), message);
}


