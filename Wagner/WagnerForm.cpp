#include "WagnerForm.h"

#pragma region UI_Events

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
		pauseEvent->Set();
		return;
	}

	if (CyclogrammTextBox->Text->Length == 0)
		return;
	if (!isScriptValid)
		return;

	FocusCommandListBox->Enabled = false;
	FocusCommandListBox->Enabled = false;
	FocusCommandListBox->Enabled = false;

	cnctToFocus->Enabled = false;
	cnctToDataFrame->Enabled = false;
	cnctToHexapod->Enabled = false;

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
	PauseButton->Enabled = false;
	StopButton->Enabled = false;
	DoCyclogrammWorker->CancelAsync();
	if (isPause)
		pauseEvent->Set();
	isPause = false;
}

System::Void Wagner::WagnerForm::WagnerForm_Load(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->AllowDrop = true;
	CyclogrammTextBox->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &Wagner::WagnerForm::OnDragDrop);
	CyclogrammTextBox->Clear();

	FocusClient = gcnew CavemanTcpClient(FocusIpPortTB->Text);
	FocusClient->Events->ClientDisconnected += gcnew System::EventHandler(this, &Wagner::WagnerForm::OnFocusDisconnected);

	HexapodClient = gcnew CavemanTcpClient(HexapodIpPortTB->Text);
	HexapodClient->Events->ClientDisconnected += gcnew System::EventHandler(this, &Wagner::WagnerForm::OnHexapodDisconnected);

	funcs->Add("getPosition", gcnew ExecuteCommand(this, &WagnerForm::GetPosition));
	funcs->Add("moveTo", gcnew ExecuteCommand(this, &WagnerForm::MoveTo));
	funcs->Add("park", gcnew ExecuteCommand(this, &WagnerForm::Park));
	funcs->Add("getErrors", gcnew ExecuteCommand(this, &WagnerForm::GetErrors));
	funcs->Add("resetErrors", gcnew ExecuteCommand(this, &WagnerForm::ResetErrors));
}

System::Void Wagner::WagnerForm::FocusCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Text += FocusCommandListBox->SelectedItem->ToString() + "()" + "\r\n";
	ValidateText();
}

System::Void Wagner::WagnerForm::DataFrameCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Text += DataFrameCommandListBox->SelectedItem->ToString() + "()" + "\r\n";
	ValidateText();
}

System::Void Wagner::WagnerForm::HexapodCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Text += HexapodCommandListBox->SelectedItem->ToString() + "()" + "\r\n";
	ValidateText();
}

System::Void Wagner::WagnerForm::cnctToFocus_Click(System::Object^ sender, System::EventArgs^ e) {
	if (FocusClient->IsConnected) {
		FocusClient->Disconnect();
	}
	else {
		this->Enabled = false;
		FocusClient->Connect(1000);
		if (FocusClient->IsConnected) {
			WagnerPacket^ packet = gcnew WagnerPacket();
			packet->command = WHO_ARE_YOU;
			packet->data = 0xD0;
			int size = Marshal::SizeOf(packet);

			auto message = getBytes(packet);

			FocusClient->Send(message);
			ReadResult^ rr = FocusClient->ReadWithTimeout(1000, size);

			if (rr->Status == ReadResultStatus::Success) {
				packet = fromBytes(rr->Data);
				if (packet->command == WHO_ARE_YOU && packet->data == 0xDD) {
					chatTextBox->Text += String::Format(gcnew String("Focus подключен\r\n"));
					cnctToFocus->ForeColor = System::Drawing::Color::Red;
					cnctToFocus->Text = "Отключить Focus";
					this->Enabled = true;
					return;
				}
			}
			chatTextBox->Text += "Попытка соединения c Focus. Авторизация не пройдена\r\n";
			FocusClient->Disconnect();
			this->Enabled = true;
		}
	}
}

System::Void Wagner::WagnerForm::cnctToDataFrame_Click(System::Object^ sender, System::EventArgs^ e) {
	// TO DO

	return;
}

System::Void Wagner::WagnerForm::cnctToHexapod_Click(System::Object^ sender, System::EventArgs^ e) {
	// TO DO
	return;
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

#pragma endregion

#pragma region Messaging

void Wagner::WagnerForm::UpdateChatBox(String^ text) {
	chatTextBox->Text += text;
}

void Wagner::WagnerForm::OnFocusDisconnected(System::Object^ sender, System::EventArgs^ e) {
	UpdateDisconnectionAction^ action = 
		gcnew UpdateDisconnectionAction(this, &Wagner::WagnerForm::UpdateClientDisconnected);
	this->Invoke(action, 0xD0);
}

void Wagner::WagnerForm::OnHexapodDisconnected(System::Object^ sender, System::EventArgs^ e) {
	UpdateDisconnectionAction^ action = 
		gcnew UpdateDisconnectionAction(this, &Wagner::WagnerForm::UpdateClientDisconnected);
	this->Invoke(action, 0XD2);
}

void Wagner::WagnerForm::UpdateClientDisconnected(int32_t app) {
	switch (app) {
	case FOCUS:
		chatTextBox->Text += "Focus Отключен\r\n";
		cnctToFocus->ForeColor = System::Drawing::Color::DarkGreen;
		cnctToFocus->Text = "Подключить Focus";
		break;
	case HEXAPOD:
		chatTextBox->Text += "Hexapod Отключен\r\n";
		cnctToHexapod->ForeColor = System::Drawing::Color::DarkGreen;
		cnctToHexapod->Text = "Подключить Hexapod";
	}
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

bool Wagner::WagnerForm::GetPosition(uint32_t data) {

	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = GET_POSITION;
	packet->data = data;

	auto message = getBytes(packet);
	if (!FocusClient->IsConnected)
		return false;
	FocusClient->Send(message);
	return true;
}

bool Wagner::WagnerForm::MoveTo(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = MOVE_TO;
	packet->data = data;

	auto message = getBytes(packet);
	if (!FocusClient->IsConnected)
		return false;
	FocusClient->Send(message);
	return true;
}

bool Wagner::WagnerForm::Park(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = PARK;
	packet->data = data;

	auto message = getBytes(packet);
	if (!FocusClient->IsConnected)
		return false;
	FocusClient->Send(message);
	return true;
}

bool Wagner::WagnerForm::GetErrors(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = GET_ERRORS;
	packet->data = data;

	auto message = getBytes(packet);
	if (!FocusClient->IsConnected)
		return false;
	FocusClient->Send(message);
	return true;
}

bool Wagner::WagnerForm::ResetErrors(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = RESET_ERRORS;
	packet->data = data;

	auto message = getBytes(packet);
	if (!FocusClient->IsConnected)
		return false;
	FocusClient->Send(message);
	return true;
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
			DoCyclogrammWorker->ReportProgress(1, packet);
			pauseEvent->WaitOne();
			continue;
		}

		try {

			String^ funcName = getFunctionFromString(commands[StepCount - 1]);
			auto args = getArgsFromString(commands[StepCount - 1]);
			bool result = funcs[funcName](args->Count == 0 ? 0 : args[0]);

			if (!result) {
				MessageBox::Show(String::Format(gcnew String("Потеряно соединение с {0}\n"), "Asa"),
					Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
				isPause = true;
				goto connectionLost;
			}

			Tasks::Task<CavemanTcp::ReadResult^>^ rr = FocusClient->ReadWithTimeoutAsync(600000, size, CancelReading->Token);
			isReading = true;
			rr->Wait();

			if (rr->Result->Status == ReadResultStatus::Success) {
				isReading = false;
				packet = fromBytes(rr->Result->Data);
				DoCyclogrammWorker->ReportProgress(1, packet);
			}
			else {
				MessageBox::Show("Прием пакета не выполнен", Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
				isPause = true;
				continue;
			}
		}
		catch (Exception^ ex) {
			MessageBox::Show(ex->Message, Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
			break;
		}

	}
}

System::Void Wagner::WagnerForm::DoCyclogrammWorker_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e) {
	if (isPause) {
		StartButton->Enabled = true;
		PauseButton->Enabled = false;
		StopButton->Enabled = true;
		return;
	}

	WagnerPacket^ packet = (WagnerPacket^)e->UserState;
	Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateChatBox);
	CyclogrammProgressBar->Increment(1);
	StepCount++;

	this->Invoke(action, String::Format(gcnew String("Focus: command:{0} data:{1}\r\n"), (Convert::ToString(packet->command)), (Convert::ToString(packet->data))));
}

System::Void Wagner::WagnerForm::DoCyclogrammWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	isReading = false;
	isPause = false;
	
	StepCount = 1;

	StartButton->Enabled = true;
	PauseButton->Enabled = false;
	StopButton->Enabled = false;


	FocusCommandListBox->Enabled = false;
	FocusCommandListBox->Enabled = false;
	FocusCommandListBox->Enabled = false;

	cnctToFocus->Enabled = false;
	cnctToDataFrame->Enabled = false;
	cnctToHexapod->Enabled = false;

	CyclogrammTextBox->ReadOnly = false;
	ClearCyclogrammButton->Enabled = true;
	StartButton->Enabled = true;
}

#pragma endregion
