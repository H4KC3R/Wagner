﻿#include "WagnerForm.h"

#pragma region UI_Events

System::Void Wagner::WagnerForm::ExpandButton_Click(System::Object^ sender, System::EventArgs^ e) {
	if (ExpandButton->Text == ">") {
		this->Width = 850;
		ExpandButton->Text = "<";
	}
	else {
		this->Width = 700;
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
	if (!ValidateText())
		return;

	FocusCommandListBox->Enabled = false;
	DataFrameCommandListBox->Enabled = false;
	HexapodCommandListBox->Enabled = false;

	cnctToFocus->Enabled = false;
	cnctToDataFrame->Enabled = false;
	cnctToHexapod->Enabled = false;

	CyclogrammTextBox->ReadOnly = true;
	ClearCyclogrammButton->Enabled = false;
	
	StartButton->Enabled = false;
	StartFromBtn->Enabled = false;
	stepCountNUD->Enabled = false;
	LoadScriptBtn->Enabled = false;

	PauseButton->Enabled = true;
	StopButton->Enabled = true;

	rxStatus = INITIAL_STATUS;
	auto commands = CyclogrammTextBox->Text->Split(gcnew array<String^>{"\r\n"}, StringSplitOptions::RemoveEmptyEntries);
	
	CyclogrammProgressBar->Maximum = commands->Length;
	CyclogrammProgressBar->BarColor = System::Drawing::Color::Green;
	CyclogrammProgressBar->Value = 0;

	progressStatusBox->Text = String::Format(gcnew String("1/{0}\r\n"), 
		System::Convert::ToString(commands->Length));
	
	DoCyclogrammWorker->RunWorkerAsync(commands);
}

System::Void Wagner::WagnerForm::PauseButton_Click(System::Object^ sender, System::EventArgs^ e) {
	isPause = true;

	if (rxStatus == ON_WAITING_MSG) {
		System::Windows::Forms::DialogResult result = MessageBox::Show("Текущая команда не завершена. Хотите ли Вы дождаться завершения команды?",
			"Отмена команды", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
		if (result == System::Windows::Forms::DialogResult::No) {
			PauseButton->Enabled = false;
			rxStatus = CANCELED;
			waitMessage->Set();
		}
	}
}

System::Void Wagner::WagnerForm::StopButton_Click(System::Object^ sender, System::EventArgs^ e) {
	DoCyclogrammWorker->CancelAsync();

	if (isPause) {
		pauseEvent->Set();
		isPause = false;
	}

	if (rxStatus == ON_WAITING_MSG) {
		System::Windows::Forms::DialogResult result = MessageBox::Show("Текущая команда не завершена. Хотите ли Вы дождаться завершения команды?",
			"Отмена команды", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
		if (result == System::Windows::Forms::DialogResult::No) {
			rxStatus = CANCELED;
			waitMessage->Set();
		}
	}
}

System::Void Wagner::WagnerForm::WagnerForm_Load(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->AllowDrop = true;
	CyclogrammTextBox->Clear();

	commands->Add("getPosition()");
	commands->Add("moveTo()");
	commands->Add("park()");
	commands->Add("getErrors()");
	commands->Add("resetErrors()");
	commands->Add("getErrors()");
	commands->Add("moveStep()");
	commands->Add("linearMove()");
	commands->Add("angularMove()");
	commands->Add("combinedMove()");
	commands->Add("moveToZero()");

	popupMenu = gcnew FastColoredTextBoxNS::AutocompleteMenu(CyclogrammTextBox);
	popupMenu->Items->Width = 200;
	popupMenu->Items->SetAutocompleteItems(commands);

	FocusClient = gcnew SimpleTcpClient(FocusIpPortTB->Text);
	FocusClient->Events->Connected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnFocusConnected);
	FocusClient->Events->Disconnected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnFocusDisconnected);
	FocusClient->Events->DataReceived += gcnew System::EventHandler<SuperSimpleTcp::DataReceivedEventArgs^>(this, &Wagner::WagnerForm::OnFocusDataReceived);

	DataFrameClient = gcnew SimpleTcpClient(DataFrameIpPortTB->Text);
	DataFrameClient->Events->Connected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnDataFrameConnected);
	DataFrameClient->Events->Disconnected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnDataFrameDisconnected);
	DataFrameClient->Events->DataReceived += gcnew System::EventHandler<SuperSimpleTcp::DataReceivedEventArgs^>(this, &Wagner::WagnerForm::OnDataFrameDataReceived);

	HexapodClient = gcnew UdpClient(HexapodIpTB->Text, Convert::ToInt64(HexapodPortTB->Text));

	connections->Add("Focus", FocusClient);
	connections->Add("DataFrame", DataFrameClient);

	funcs->Add("getPosition", gcnew ExecuteCommand(this, &WagnerForm::GetPosition));
	funcs->Add("moveTo", gcnew ExecuteCommand(this, &WagnerForm::MoveTo));
	funcs->Add("park", gcnew ExecuteCommand(this, &WagnerForm::Park));
	funcs->Add("getErrors", gcnew ExecuteCommand(this, &WagnerForm::GetErrors));
	funcs->Add("resetErrors", gcnew ExecuteCommand(this, &WagnerForm::ResetErrors));
	funcs->Add("moveStep", gcnew ExecuteCommand(this, &WagnerForm::moveStep));
}

System::Void Wagner::WagnerForm::FocusCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Text += FocusCommandListBox->SelectedItem->ToString() + "()" + "\r\n";
}

System::Void Wagner::WagnerForm::DataFrameCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Text += DataFrameCommandListBox->SelectedItem->ToString() + "()" + "\r\n";
}

System::Void Wagner::WagnerForm::HexapodCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Text += HexapodCommandListBox->SelectedItem->ToString() + "()" + "\r\n";
}

System::Void Wagner::WagnerForm::cnctToFocus_Click(System::Object^ sender, System::EventArgs^ e) {
	if (FocusClient->IsConnected)
		FocusClient->Disconnect();
	else {
		try {
			FocusClient->Connect();
		}
		catch (Exception^ ex) {
			MessageBox::Show(ex->Message, Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
	}
}

System::Void Wagner::WagnerForm::cnctToDataFrame_Click(System::Object^ sender, System::EventArgs^ e) {
	if (DataFrameClient->IsConnected)
		DataFrameClient->Disconnect();
	else {
		try {
			DataFrameClient->Connect();
		}
		catch (Exception^ ex) {
			MessageBox::Show(ex->Message, Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
	}
}

System::Void Wagner::WagnerForm::cnctToHexapod_Click(System::Object^ sender, System::EventArgs^ e) {
	return;
}

System::Void Wagner::WagnerForm::ClearCyclogrammButton_Click(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Clear();
}

System::Void Wagner::WagnerForm::SaveScriptBtn_Click(System::Object^ sender, System::EventArgs^ e) {
	if (saveScriptFile->ShowDialog() == System::Windows::Forms::DialogResult::Cancel)
		return;

	String^ filename = saveScriptFile->FileName;
	System::IO::File::WriteAllText(filename, CyclogrammTextBox->Text);
}

System::Void Wagner::WagnerForm::LoadScriptBtn_Click(System::Object^ sender, System::EventArgs^ e) {
	if (openScriptFile->ShowDialog() == System::Windows::Forms::DialogResult::Cancel)
		return;

	String^ filename = openScriptFile->FileName;
	String^ fileText = System::IO::File::ReadAllText(filename);
	CyclogrammTextBox->Text = fileText;
	ValidateText();
}

System::Void Wagner::WagnerForm::CyclogrammTextBox_TextChanged(System::Object^ sender, FastColoredTextBoxNS::TextChangedEventArgs^ e) {
	String^ pattern = "^getPosition[(][)]|^park[(][)]|^getErrors[(][)]|^resetErrors[(][)]|moveStep[(]?[0-9]+[.]?[0-9]+[)]?|moveTo[(][0-9]+[)]";
	e->ChangedRange->ClearStyle(BlueStyle);
	e->ChangedRange->SetStyle(BlueStyle, pattern, RegexOptions::Multiline);
}

System::Void Wagner::WagnerForm::ReplaceBtn_Click(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->ShowReplaceDialog();
}

System::Void Wagner::WagnerForm::FindBtn_Click(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->ShowFindDialog();
}

System::Void Wagner::WagnerForm::ClearMessageChatBtn_Click(System::Object^ sender, System::EventArgs^ e) {
	chatTextBox->Clear();
}

#pragma endregion

#pragma region Messaging

void Wagner::WagnerForm::UpdateChatBox(String^ text) {
	chatTextBox->Text += text;
}

void Wagner::WagnerForm::UpdateFocusConnected() {
	chatTextBox->Text += "Focus подключен\r\n";
	cnctToFocus->ForeColor = System::Drawing::Color::Red;
	cnctToFocus->Text = "Отключить Focus";
}

void Wagner::WagnerForm::UpdateFocusDisconnected() {
	chatTextBox->Text += "Focus Отключен\r\n";
	cnctToFocus->ForeColor = System::Drawing::Color::DarkGreen;
	cnctToFocus->Text = "Подключить Focus";

	if (rxStatus == ON_WAITING_MSG) {
		rxStatus = CANCELED;
		waitMessage->Set();
	}
	isPause = true;
}

void Wagner::WagnerForm::OnFocusConnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	UpdateAction^ action = gcnew UpdateAction(this, &Wagner::WagnerForm::UpdateFocusConnected);
	this->BeginInvoke(action);
}

void Wagner::WagnerForm::OnFocusDisconnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	UpdateAction^ action = gcnew UpdateAction(this, &Wagner::WagnerForm::UpdateFocusDisconnected);
	this->BeginInvoke(action);
}

void Wagner::WagnerForm::OnFocusDataReceived(System::Object^ sender, SuperSimpleTcp::DataReceivedEventArgs^ e) {
	auto txMsg = fromBytes(e->Data);
	if (rxStatus = ON_WAITING_MSG) {
		rxStatus = READING_PACKET;
		ParsePackets^ action = gcnew ParsePackets(this, &WagnerForm::ParsePacket);
		this->BeginInvoke(action, txMsg);
	}
}

void Wagner::WagnerForm::UpdateDataFrameConnected() {
	chatTextBox->Text += "DataFrame подключен\r\n";
	cnctToDataFrame->ForeColor = System::Drawing::Color::Red;
	cnctToDataFrame->Text = "Отключить DataFrame";
}

void Wagner::WagnerForm::UpdateDataFrameDisconnected() {
	chatTextBox->Text += "DataFrame Отключен\r\n";
	cnctToDataFrame->ForeColor = System::Drawing::Color::DarkGreen;
	cnctToDataFrame->Text = "Подключить DataFrame";
}

void Wagner::WagnerForm::OnDataFrameConnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	UpdateAction^ action = gcnew UpdateAction(this, &Wagner::WagnerForm::UpdateDataFrameConnected);
	this->Invoke(action);
}

void Wagner::WagnerForm::OnDataFrameDisconnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	UpdateAction^ action = gcnew UpdateAction(this, &Wagner::WagnerForm::UpdateDataFrameDisconnected);
	this->Invoke(action);
}

void Wagner::WagnerForm::OnDataFrameDataReceived(System::Object^ sender, SuperSimpleTcp::DataReceivedEventArgs^ e) {
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

System::String^ Wagner::WagnerForm::getFunctionNameFromString(String^ s) {
	int charLocation = s->IndexOf("(", StringComparison::Ordinal);
	if (charLocation > 0) {
		return s->Substring(0, charLocation);
	}
	return String::Empty;
}

System::Collections::Generic::List<uint32_t>^ Wagner::WagnerForm::getFunctionArgsFromString(String^ s) {
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

bool Wagner::WagnerForm::isFunctionValid(String^ s) {
	bool isCorrectName = false;
	bool isCorrectArgs = false;
	int startOfArgs = s->IndexOf("(") + 1;
	int endOfArgs = s->IndexOf(")");
	if (s->Length - endOfArgs != 1)
		return false;

	String^ funcName = getFunctionNameFromString(s);
	if (String::IsNullOrEmpty(funcName))
		return false;	
	
	if (funcs->ContainsKey(funcName))
		isCorrectName = true;
	
	auto args = getFunctionArgsFromString(s);
	if ((funcName == "moveTo" || funcName == "moveStep" ) && args->Count == 1)
		isCorrectArgs = true;
	else if (isCorrectName && funcName != "moveTo" && (endOfArgs - startOfArgs) == 0)
		isCorrectArgs = true;
	return (isCorrectName && isCorrectArgs);
}

System::String^ Wagner::WagnerForm::getAppByFuncName(String^ funcName) {
	if (FocusCommandListBox->Items->Contains(funcName))
		return "Focus";
	else if (DataFrameCommandListBox->Items->Contains(funcName))
		return "DataFrame";
	else if (HexapodCommandListBox->Items->Contains(funcName))
		return "Hexapod";
}

bool Wagner::WagnerForm::ValidateText() {
	System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::WaitCursor;
	auto commands = CyclogrammTextBox->Text->Split(gcnew array<String^>{"\r\n"}, StringSplitOptions::RemoveEmptyEntries);
	
	CyclogrammTextBox->Clear();
	bool isScriptValid = true;

	for (int i = 0; i < commands->Length; i++) {
		if (!isFunctionValid(commands[i])) {
			CyclogrammTextBox->AppendText(commands[i] + "\r\n", RedStyle);
			isScriptValid = false;
		}
		else
			CyclogrammTextBox->AppendText(commands[i] + "\r\n");
	}

	System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::Default;
	return isScriptValid;
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
	rxStatus = ON_WAITING_MSG;
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

	rxStatus = ON_WAITING_MSG;
	FocusClient->Send(message);
	return true;
}

bool Wagner::WagnerForm::Park(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();

	packet->command = MOVE_TO;
	packet->data = 1000;

	auto message = getBytes(packet);
	if (!FocusClient->IsConnected)
		return false;

	rxStatus = ON_WAITING_MSG;
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

	rxStatus = ON_WAITING_MSG;
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

	rxStatus = ON_WAITING_MSG;
	FocusClient->Send(message);
	return true;
}

bool Wagner::WagnerForm::moveStep(uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();
	
	packet->command = MOVE_STEP;
	packet->data = data;

	auto message = getBytes(packet);
	if (!FocusClient->IsConnected)
		return false;

	rxStatus = ON_WAITING_MSG;
	FocusClient->Send(message);
	return true;
}

bool Wagner::WagnerForm::ParsePacket(WagnerPacket^ packet) {
	if (packet->command == GET_POSITION) {
		float position = ((float)packet->data) / 1000.0;
		chatTextBox->Text += String::Format(gcnew String("Текущая позиция {0}\r\n"), System::Convert::ToString(position));
		waitMessage->Set();
		return true;
	}
	else if (packet->command == MOVE_TO) {
		switch (packet->data) {
		case APP_IS_BUSY:
			chatTextBox->Text += "Wagner не может получить доступ к FocusAPP, так как это приложение занято. Повторите попытку.\r\n";
			waitMessage->Set();
			return false;
		case REF_POINT_NOT_CAPTURED:
			chatTextBox->Text += "Референтная метка не захвачена. Захватите реф.метку через FocusAPP и повторите попытку.\r\n";
			waitMessage->Set();
			return false;
		case SENSOR_ERROR:
			chatTextBox->Text += "Ошибка датчика.Проверьте соединение и повторите попытку.\r\n";
			waitMessage->Set();
			return false;
		case STOPPED_IN_FOCUSAPP:
			chatTextBox->Text += "Движение в точку остановлено.\r\n";
			waitMessage->Set();
			return false;
		case POSITIONING_ERROR:
			chatTextBox->Text += "Ошибка позиционирования.\r\n";
			waitMessage->Set();
			return false;
		case WRONG_DIRECTION:
			chatTextBox->Text += "Ошибка направления.\r\n";
			waitMessage->Set();
			return false;
		case NOT_MOVING:
			chatTextBox->Text += "Нет движения.\r\n";
			waitMessage->Set();
			return false;
		case UNKNOWN_COMMAND:
			chatTextBox->Text += "Неизвестная команда.\r\n";
			waitMessage->Set();
			return false;
		default:
			float position = ((float)packet->data) / 1000.0;
			chatTextBox->Text += String::Format(gcnew String("Текущая позиция {0}\r\n"), System::Convert::ToString(position));
			waitMessage->Set();
			return true;
		}
	}
	else if (packet->command == GET_ERRORS) {
		uint32_t ammountOfError = packet->data;
		chatTextBox->Text += String::Format(gcnew String("Количество ошибок {0}.\r\n"), System::Convert::ToString(ammountOfError));
		waitMessage->Set();
		return true;
	}
	else if (packet->command == RESET_ERRORS) {
		switch (packet->data) {
		case APP_IS_BUSY:
			chatTextBox->Text += "Wagner не может получить доступ к FocusAPP, так как это приложение занято. Повторите попытку.\r\n";
			waitMessage->Set();
			return false;
		default:
			chatTextBox->Text += "Ошибки сброшены.\r\n";
			waitMessage->Set();
			return true;
		}
	}
	else {
		chatTextBox->Text += "Неверный формат пакета.\r\n";
		waitMessage->Set();
		return false;
	}
}

#pragma endregion

#pragma region Cyclogramm

void Wagner::WagnerForm::OnPausedHandler() {
	CyclogrammProgressBar->BarColor = System::Drawing::Color::Yellow;

	StartButton->Enabled = true;
	PauseButton->Enabled = false;
	StopButton->Enabled = true;
}

System::Void Wagner::WagnerForm::DoCyclogrammWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	WagnerPacket^ packet = gcnew WagnerPacket();
	int size = Marshal::SizeOf(packet);
	auto commands = (array<System::String^>^)e->Argument;

	while (StepCount <= commands->Length) {
		if (DoCyclogrammWorker->CancellationPending)
			break;
	
	connectionLost:
		if (isPause) {
			UpdateAction^ pause = gcnew UpdateAction(this, &Wagner::WagnerForm::OnPausedHandler);
			this->Invoke(pause);
			pauseEvent->WaitOne();
			continue;
		}

		try {
			String^ funcName = getFunctionNameFromString(commands[StepCount - 1]);
			auto args = getFunctionArgsFromString(commands[StepCount - 1]);
			String^ app = getAppByFuncName(funcName);

			bool isConnected = funcs[funcName](args->Count == 0 ? 0 : args[0]);
			if (!isConnected) {
				MessageBox::Show(String::Format(gcnew String("Нет соединения с {0}\n"), app),
					Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
				isPause = true;
				goto connectionLost;
			}
			waitMessage->WaitOne(600000);

			if (rxStatus == ON_WAITING_MSG) {
				rxStatus = INITIAL_STATUS;
				isPause = true;
				Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateChatBox);
				this->Invoke(action, "Таймаут запроса\r\n");
			}
			else if (rxStatus == CANCELED) {
				Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateChatBox);
				this->Invoke(action, "Запрос отменен\r\n");
			}
			else {
				StepCount++;
				DoCyclogrammWorker->ReportProgress(1, commands->Length);
			}
		}
		catch (Exception^ ex) {
			MessageBox::Show(ex->Message, Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
			break;
		}

	}
}

System::Void Wagner::WagnerForm::DoCyclogrammWorker_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e) {
	CyclogrammProgressBar->Increment(1);
	int TotalStep = (int)e->UserState;
		progressStatusBox->Text = String::Format(gcnew String("{0}/{1}\r\n"), 
			System::Convert::ToString(StepCount),
			System::Convert::ToString(TotalStep));
}

System::Void Wagner::WagnerForm::DoCyclogrammWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	rxStatus = INITIAL_STATUS;
	isPause = false;
	
	StepCount = 1;

	StartButton->Enabled = true;
	StartFromBtn->Enabled = true;
	stepCountNUD->Enabled = true;
	LoadScriptBtn->Enabled = true;

	CyclogrammProgressBar->BarColor = System::Drawing::Color::Red;

	PauseButton->Enabled = false;
	StopButton->Enabled = false;

	FocusCommandListBox->Enabled = true;
	FocusCommandListBox->Enabled = true;
	FocusCommandListBox->Enabled = true;

	cnctToFocus->Enabled = true;
	cnctToDataFrame->Enabled = true;
	cnctToHexapod->Enabled = true;

	CyclogrammTextBox->ReadOnly = false;
	ClearCyclogrammButton->Enabled = true;
}

#pragma endregion