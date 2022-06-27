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
	if (isPause) {
		StartButton->Enabled = false;
		PauseButton->Enabled = true;
		StopButton->Enabled = true;

		isPause = false;
		return;
	}

	if (CyclogrammTextBox->Text->Length == 0)
		return;
	if (!isScriptValid)
		return;
	if (clientsDictionary->Count == 0)
		return;

	CommandListBox->Enabled = false;
	CyclogrammTextBox->ReadOnly = true;
	ClearCyclogrammButton->Enabled = false;
	StartButton->Enabled = false;
	PauseButton->Enabled = true;
	StopButton->Enabled = true;

	auto commands = CyclogrammTextBox->Text->Split(gcnew array<String^>{"\n"}, StringSplitOptions::RemoveEmptyEntries);
	CyclogrammProgressBar->Maximum = commands->Length;

	DoCyclogrammWorker->RunWorkerAsync(commands);
}

System::Void Wagner::WagnerForm::PauseButton_Click(System::Object^ sender, System::EventArgs^ e) {
	if (isReading) {
		System::Windows::Forms::DialogResult result = MessageBox::Show("Текущая команда не завершена. Хотите ли Вы дождаться завершения команды?",
			"Отмена команды", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
		if (result == System::Windows::Forms::DialogResult::No)
			CancelReading->Cancel();
	}
	PauseButton->Enabled = false;
	isPause = true;
}

System::Void Wagner::WagnerForm::StopButton_Click(System::Object^ sender, System::EventArgs^ e) {
	if (isReading) {
		System::Windows::Forms::DialogResult result = MessageBox::Show("Текущая команда не завершена. Хотите ли Вы дождаться завершения команды?",
			"Отмена команды", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
		if (result == System::Windows::Forms::DialogResult::No)
			CancelReading->Cancel();
	}
	DoCyclogrammWorker->CancelAsync();
}

System::Void Wagner::WagnerForm::WagnerForm_Load(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->AllowDrop = true;
	CyclogrammTextBox->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &Wagner::WagnerForm::OnDragDrop);
	CyclogrammTextBox->Clear();

	server = gcnew CavemanTcpServer(ServerAdrress->Text);

	server->Events->ClientConnected += gcnew System::EventHandler<CavemanTcp::ClientConnectedEventArgs^>(this, &Wagner::WagnerForm::OnClientConnected);
	server->Events->ClientDisconnected += gcnew System::EventHandler<CavemanTcp::ClientDisconnectedEventArgs^>(this, &Wagner::WagnerForm::OnClientDisconnected);
	server->Start();
	chatTextBox->Text += "Запуск....\r\n";

	funcs->Add("getPosition", gcnew Action<uint32_t>(this, &WagnerForm::GetPosition));
	funcs->Add("moveTo", gcnew Action<uint32_t>(this, &WagnerForm::MoveTo));
	funcs->Add("park", gcnew Action<uint32_t>(this, &WagnerForm::Park));
	funcs->Add("getErrors", gcnew Action<uint32_t>(this, &WagnerForm::GetErrors));
	funcs->Add("resetErrors", gcnew Action<uint32_t>(this, &WagnerForm::ResetErrors));
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
	this->Enabled = false;

	WagnerPacket^ packet = gcnew WagnerPacket();
	packet->command = WHO_ARE_YOU;
	packet->data = 0;
	int size = Marshal::SizeOf(packet);

	auto message = getBytes(packet);

	server->Send(ip, message);
	ReadResult^ rr = server->ReadWithTimeout(1000, ip, size);

	if (rr->Status == ReadResultStatus::Success) {
		packet = fromBytes(rr->Data);
		if (packet->command == WHO_ARE_YOU && packet->data == 0) {
			clientsDictionary->Add(ip, "Focus");
			chatTextBox->Text += String::Format(gcnew String("{0}: подключен\r\n"), ip);
			clientsListBox->Items->Add("Focus");
			this->Enabled = true;
			return;
		}
	}
	this->Enabled = true;
	chatTextBox->Text += String::Format(gcnew String("Попытка соединения {0}. Авторизация не пройдена\r\n"), ip);
	server->DisconnectClient(ip);
}

void Wagner::WagnerForm::UpdateClientDisconnected(String^ ip) {
	if (clientsDictionary->ContainsKey(ip)) {
		chatTextBox->Text += String::Format(gcnew String("{0}: отключен\r\n"), ip);
		clientsListBox->Items->Remove(clientsDictionary[ip]);
	}
}

void Wagner::WagnerForm::OnClientConnected(System::Object^ sender, CavemanTcp::ClientConnectedEventArgs^ e) {
	Update^ action = gcnew Wagner::WagnerForm::Update(this, &Wagner::WagnerForm::UpdateClientConnected);
	this->Invoke(action, e->IpPort);
}

void Wagner::WagnerForm::OnClientDisconnected(System::Object^ sender, CavemanTcp::ClientDisconnectedEventArgs^ e) {
	Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateClientDisconnected);
	this->Invoke(action, e->IpPort);
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
	
	if (funcs->ContainsKey(funcName))
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

#pragma region Funcs

void Wagner::WagnerForm::GetPosition(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = GET_POSITION;
	packet->data = data;

	auto message = getBytes(packet);
	server->Send(clientsListBox->SelectedItem->ToString(), message);
}

void Wagner::WagnerForm::MoveTo(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = MOVE_TO;
	packet->data = data;

	auto message = getBytes(packet);
	server->Send(clientsListBox->SelectedItem->ToString(), message);
}

void Wagner::WagnerForm::Park(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = PARK;
	packet->data = data;

	auto message = getBytes(packet);
	server->Send(clientsListBox->SelectedItem->ToString(), message);
}

void Wagner::WagnerForm::GetErrors(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = GET_ERRORS;
	packet->data = data;

	auto message = getBytes(packet);
	server->Send(clientsListBox->SelectedItem->ToString(), message);
}

void Wagner::WagnerForm::ResetErrors(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = RESET_ERRORS;
	packet->data = data;

	auto message = getBytes(packet);
	server->Send(clientsListBox->SelectedItem->ToString(), message);
}

#pragma endregion

#pragma region Cyclogramm

System::Void Wagner::WagnerForm::DoCyclogrammWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	WagnerPacket^ packet = gcnew WagnerPacket();
	int size = Marshal::SizeOf(packet);
	auto commands = (array<System::String^>^)e->Argument;

	while (StepCount < commands->Length) {
		if (DoCyclogrammWorker->CancellationPending)
			break;
	
	connectionLost:
		if (isPause) {
			DoCyclogrammWorker->ReportProgress(1);
			continue;
		}

		for each (KeyValuePair<String^, String^>^ kvp in clientsDictionary) {
			if (!server->IsConnected(kvp->Value)) {
				MessageBox::Show(String::Format(gcnew String("Потеряно соединение с {0}\n"), kvp->Value),
					Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
				isPause = true;
				goto connectionLost;
			}
		}

		String^ funcName = getFunctionFromString(commands[StepCount]);
		auto args = getArgsFromString(commands[StepCount]);

		Tasks::Task<CavemanTcp::ReadResult^>^ rr = server->ReadWithTimeoutAsync(600000, clientsDictionary["Focus"], size, CancelReading->Token);
		isReading = true;
		rr->Wait();

		if (rr->Result->Status == ReadResultStatus::Success) {
			packet = fromBytes(rr->Result->Data);
			DoCyclogrammWorker->ReportProgress(1, packet);
			
		}
	}
}

System::Void Wagner::WagnerForm::DoCyclogrammWorker_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e) {
	if (isPause) {
		StartButton->Enabled = true;
		StopButton->Enabled = true;
		return;
	}

	WagnerPacket^ packet = (WagnerPacket^)e->UserState;
	Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateChatBox);
	CyclogrammProgressBar->Increment(1);
	StepCount++;

	this->Invoke(action, String::Format(gcnew String("Focus: command:{0} data:{1}\r\n"), (Convert::ToString(packet->command)), (Convert::ToString(packet->data))));
}

#pragma endregion



