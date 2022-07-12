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
	if (CyclogrammTextBox->Text->Length == 0)
		return;
	if (!isScriptValid)
		return;

	if (isPause) {
		StartButton->Enabled = false;
		PauseButton->Enabled = true;
		StopButton->Enabled = true;

		isPause = false;
		pauseEvent->Set();
		return;
	}
	FocusCommandListBox->Enabled = false;
	DataFrameCommandListBox->Enabled = false;
	HexapodCommandListBox->Enabled = false;

	cnctToFocus->Enabled = false;
	cnctToDataFrame->Enabled = false;
	cnctToHexapod->Enabled = false;

	CyclogrammTextBox->ReadOnly = true;
	ClearCyclogrammButton->Enabled = false;
	StartButton->Enabled = false;
	PauseButton->Enabled = true;
	StopButton->Enabled = true;

	rxStatus = INITIAL_STATUS;
	auto commands = CyclogrammTextBox->Text->Split(gcnew array<String^>{"\n"}, StringSplitOptions::RemoveEmptyEntries);
	CyclogrammProgressBar->Maximum = commands->Length;

	DoCyclogrammWorker->RunWorkerAsync(commands);
}

System::Void Wagner::WagnerForm::PauseButton_Click(System::Object^ sender, System::EventArgs^ e) {
	isPause = true;
	PauseButton->Enabled = false;

	if (rxStatus = ON_WAITING_MSG) {
		System::Windows::Forms::DialogResult result = MessageBox::Show("Текущая команда не завершена. Хотите ли Вы дождаться завершения команды?",
			"Отмена команды", MessageBoxButtons::YesNo, MessageBoxIcon::Question);
		if (result == System::Windows::Forms::DialogResult::No) {
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

	if (rxStatus = ON_WAITING_MSG) {
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
	CyclogrammTextBox->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &Wagner::WagnerForm::OnDragDrop);
	CyclogrammTextBox->Clear();

	FocusClient = gcnew SimpleTcpClient(FocusIpPortTB->Text);
	FocusClient->Events->Connected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnFocusConnected);
	FocusClient->Events->Disconnected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnFocusDisconnected);
	FocusClient->Events->DataReceived += gcnew System::EventHandler<SuperSimpleTcp::DataReceivedEventArgs^>(this, &Wagner::WagnerForm::OnFocusDataReceived);

	DataFrameClient = gcnew SimpleTcpClient(DataFrameIpPortTB->Text);
	DataFrameClient->Events->Connected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnDataFrameConnected);
	DataFrameClient->Events->Disconnected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnDataFrameDisconnected);
	DataFrameClient->Events->DataReceived += gcnew System::EventHandler<SuperSimpleTcp::DataReceivedEventArgs^>(this, &Wagner::WagnerForm::OnDataFrameDataReceived);

	HexapodClient = gcnew UdpClient(HexapodIpTB->Text, Convert::ToInt64(HexapodPortTB->Text));

	FocusFuncs->Add("getPosition");
	FocusFuncs->Add("moveTo");
	FocusFuncs->Add("park");
	FocusFuncs->Add("getErrors");
	FocusFuncs->Add("resetErrors");

	connections->Add("Focus", FocusClient);
	connections->Add("DataFrame", DataFrameClient);

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

void Wagner::WagnerForm::UpdateFocusConnected() {
	chatTextBox->Text += "Focus подключен\r\n";
	cnctToFocus->ForeColor = System::Drawing::Color::Red;
	cnctToFocus->Text = "Отключить Focus";
}

void Wagner::WagnerForm::UpdateFocusDisconnected() {
	chatTextBox->Text += "Focus Отключен\r\n";
	cnctToFocus->ForeColor = System::Drawing::Color::DarkGreen;
	cnctToFocus->Text = "Подключить Focus";
}

void Wagner::WagnerForm::OnFocusConnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	UpdateAction^ action = gcnew UpdateAction(this, &Wagner::WagnerForm::UpdateFocusConnected);
	this->Invoke(action);
}

void Wagner::WagnerForm::OnFocusDisconnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	UpdateAction^ action = gcnew UpdateAction(this, &Wagner::WagnerForm::UpdateFocusDisconnected);
	this->Invoke(action);
}

void Wagner::WagnerForm::OnFocusDataReceived(System::Object^ sender, SuperSimpleTcp::DataReceivedEventArgs^ e) {
	auto txMsg = fromBytes(e->Data);
	if (rxStatus = ON_WAITING_MSG) {
		rxStatus = READING_PACKET;
		ParsePackets^ action = gcnew ParsePackets(this, &WagnerForm::ParsePacket);
		this->Invoke(action);
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

bool Wagner::WagnerForm::isFunctionValid(String^ s) {
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
		if (!isFunctionValid(textLines[i])) {
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

System::String^ Wagner::WagnerForm::getAppByFuncName(String^ funcName) {
	if (FocusFuncs->Contains(funcName))
		return "Focus";
	else if(DataFrameFuncs->Contains(funcName))
		return "DataFrame";
	else if (HexapodFuncs->Contains(funcName))
		return "Hexapod";
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

bool Wagner::WagnerForm::ParsePacket(WagnerPacket^ packet) {
	if (packet->command == GET_POSITION) {
		float position = ((float)packet->data) / 1000.0;
		chatTextBox->Text += String::Format(gcnew String("Текущая позиция {0}\n"), System::Convert::ToString(position));
		waitMessage->Set();
		return true;
	}
	else if (packet->command == MOVE_TO) {
		switch (packet->data) {
		case APP_IS_BUSY:
			chatTextBox->Text += "Wagner не может получить доступ к FocusAPP, так как это приложение занято. Повторите попытку.\n";
			waitMessage->Set();
			return false;
		case REF_POINT_NOT_CAPTURED:
			chatTextBox->Text += "Референтная метка не захвачена. Захватите реф.метку через FocusAPP и повторите попытку.\n";
			waitMessage->Set();
			return false;
		case SENSOR_ERROR:
			chatTextBox->Text += "Ошибка датчика.Проверьте соединение и повторите попытку.\n";
			waitMessage->Set();
			return false;
		case STOPPED_IN_FOCUSAPP:
			chatTextBox->Text += "Движение в точку остановлено.\n";
			waitMessage->Set();
			return false;
		case POSITIONING_ERROR:
			chatTextBox->Text += "Ошибка позиционирования.\n";
			waitMessage->Set();
			return false;
		case WRONG_DIRECTION:
			chatTextBox->Text += "Ошибка направления.\n";
			waitMessage->Set();
			return false;
		case NOT_MOVING:
			chatTextBox->Text += "Нет движения.\n";
			waitMessage->Set();
			return false;
		case UNKNOWN_COMMAND:
			chatTextBox->Text += "Неизвестная команда.\n";
			waitMessage->Set();
			return false;
		default:
			float position = ((float)packet->data) / 1000.0;
			chatTextBox->Text += String::Format(gcnew String("Текущая позиция {0}\n"), System::Convert::ToString(position));
			waitMessage->Set();
			return true;
		}
	}
	else if (packet->command == GET_ERRORS) {
		uint32_t ammountOfError = packet->data;
		chatTextBox->Text += String::Format(gcnew String("Количество ошибок {0}\n"), System::Convert::ToString(ammountOfError));
		waitMessage->Set();
		return true;
	}
	else if (packet->command == RESET_ERRORS) {
		switch (packet->data) {
		case APP_IS_BUSY:
			chatTextBox->Text += "Wagner не может получить доступ к FocusAPP, так как это приложение занято. Повторите попытку.\n";
			waitMessage->Set();
			return false;
		default:
			chatTextBox->Text += "Ошибки сброшены.\n";
			waitMessage->Set();
			return true;
		}
	}
	else {
		chatTextBox->Text += "Неверный формат пакета.\n";
		waitMessage->Set();
		return false;
	}
}

#pragma endregion

#pragma region Cyclogramm

void Wagner::WagnerForm::OnPausedHandler() {
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
			String^ funcName = getFunctionFromString(commands[StepCount - 1]);
			auto args = getArgsFromString(commands[StepCount - 1]);
			String^ app = getAppByFuncName(funcName);

			bool isConnected = funcs[funcName](args->Count == 0 ? 0 : args[0]);
			if (!isConnected) {
				MessageBox::Show(String::Format(gcnew String("Потеряно соединение с {0}\n"), app),
					Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
				isPause = true;
				goto connectionLost;
			}
			waitMessage->WaitOne(600000);

			if (rxStatus == ON_WAITING_MSG) {
				isPause = true;
				Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateChatBox);
				this->Invoke(action, "Таймаут запроса");
			}
			else if (rxStatus == CANCELED) {
				Update^ action = gcnew Update(this, &Wagner::WagnerForm::UpdateChatBox);
				this->Invoke(action, "Запрос отменен");
			}
			else {
				StepCount++;
				DoCyclogrammWorker->ReportProgress(1);
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
}

System::Void Wagner::WagnerForm::DoCyclogrammWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	rxStatus = INITIAL_STATUS;
	isPause = false;
	
	StepCount = 1;

	StartButton->Enabled = true;
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
