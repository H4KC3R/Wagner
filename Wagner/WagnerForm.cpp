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
	StartButton->Enabled = false;
	server->SendAndWait(5000, client, "HAizenber", nullptr);
	StartButton->Enabled = true;

	//if (CyclogrammTextBox->Text->Length == 0)
	//	return;
	//if (!isScriptValid)
	//	return;
}

System::Void Wagner::WagnerForm::WagnerForm_Load(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->AllowDrop = true;
	CyclogrammTextBox->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &Wagner::WagnerForm::OnDragDrop);
	CyclogrammTextBox->Clear();

	server = gcnew WatsonTcpServer(ServerAddress->Text, 9000);
	server->Events->ClientConnected += gcnew System::EventHandler<WatsonTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnClientConnected);
	server->Events->ClientDisconnected += gcnew System::EventHandler<WatsonTcp::DisconnectionEventArgs^>(this, &Wagner::WagnerForm::OnClientDisconnected);
	server->Events->MessageReceived += gcnew System::EventHandler<WatsonTcp::MessageReceivedEventArgs^>(this, &Wagner::WagnerForm::OnMessageReceived);

	//Func<WatsonTcp::SyncRequest^, WatsonTcp::SyncResponse^>^ SyncRequestReceived = gcnew Func<WatsonTcp::SyncRequest^, WatsonTcp::SyncResponse^>(this,
	//	&Wagner::WagnerForm::SyncRequestReceived);
	//server->Callbacks->SyncRequestReceived = SyncRequestReceived;
	server->Start();
	chatTextBox->Text += "Запуск....\r\n";

	funcDictionary->Add("getPosition", GET_POSITION);
	funcDictionary->Add("moveTo", MOVE_TO);
	funcDictionary->Add("park", PARK);
	funcDictionary->Add("getErrors", GET_ERRORS);
	funcDictionary->Add("resetErrors", RESET_ERRORS);
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

void Wagner::WagnerForm::UpdateClientConnected(String^ text) {
	chatTextBox->Text += String::Format(gcnew String("{0}: подключен\r\n"), text);
	clientsListBox->Items->Add(text);
}

void Wagner::WagnerForm::UpdateClientDisconnected(String^ text) {
	chatTextBox->Text += String::Format(gcnew String("{0}: отключен\r\n"), text);
	clientsListBox->Items->Remove(text);
}

void Wagner::WagnerForm::OnClientConnected(System::Object^ sender, WatsonTcp::ConnectionEventArgs^ e) {
	client = e->IpPort;
	//try {
	//	WagnerPacket^ packet = gcnew WagnerPacket();
	//	packet->command = WHO_ARE_YOU;
	//	packet->data = 0;
	//	auto message = getBytes(packet);
	//	server->SendAndWait(5000, e->IpPort, "HAizenber", nullptr);

	//	//SyncResponse^ resp = server->SendAndWait(5000, e->IpPort, "HAizenber", nullptr);
	//	//packet = fromBytes(resp->Data);
	//	//if (packet->command == WHO_ARE_YOU && packet->data == 0) {
	//	//	clientsDictionary->Add(e->IpPort, "Focus");
	//	//	Update^ action = gcnew Wagner::WagnerForm::Update(this, &Wagner::WagnerForm::UpdateClientConnected);
	//	//	this->Invoke(action, "Focus");
	//	//}
	//}
	//catch (TimeoutException^ ex) {
	//	server->DisconnectClient(e->IpPort, MessageStatus::AuthFailure, true);
	//}
}

void Wagner::WagnerForm::OnClientDisconnected(System::Object^ sender, WatsonTcp::DisconnectionEventArgs^ e) {
	if (clientsDictionary->ContainsKey(e->IpPort)) {
		Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateClientDisconnected);
		this->Invoke(action, e->IpPort);
	}
	else {
		Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateChatBox);
		this->Invoke(action, String::Format(gcnew String("Попытка соединения {0}. Авторизация не пройдена\r\n"), e->IpPort));
	}
}

void Wagner::WagnerForm::OnMessageReceived(System::Object^ sender, WatsonTcp::MessageReceivedEventArgs^ e) {
	throw gcnew System::NotImplementedException();
}

WatsonTcp::SyncResponse^ Wagner::WagnerForm::SyncRequestReceived(SyncRequest^ req) {
	Console::WriteLine("va");
	
	return nullptr;
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

void Wagner::WagnerForm::sendPacket(String^ ipPort, uint8_t func, uint32_t dataToSend) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = func;
	packet->data = dataToSend;
	
	auto message = getBytes(packet);
	server->SendAndWait(1000, ipPort, message, nullptr, 0);

}