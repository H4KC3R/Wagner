#include "WagnerForm.h"

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

System::Void Wagner::WagnerForm::StartButton_Click(System::Object^ sender, System::EventArgs^ e)  {
	if (isPause) {
		StartButton->Enabled = false;
		PauseButton->Enabled = true;
		StopButton->Enabled = true;

		CyclogrammProgressBar->BarColor = System::Drawing::Color::Green;

		isPause = false;
		pauseEvent->Set();
		return;
	}

	if (CyclogrammTextBox->Text->Length == 0)
		return;
	if (!ValidateText())
		return;

	FocusCommandListBox->Enabled = false;
	MegaFrameCommandListBox->Enabled = false;
	HexapodCommandListBox->Enabled = false;

	cnctToFocus->Enabled = false;
	cnctToMegaFrame->Enabled = false;
	cnctToHexapod->Enabled = false;

	CyclogrammTextBox->ReadOnly = true;
	ClearCyclogrammButton->Enabled = false;
	
	StartButton->Enabled = false;
	stepCountNUD->Enabled = false;
	LoadScriptBtn->Enabled = false;

	PauseButton->Enabled = true;
	StopButton->Enabled = true;

	StartButton->Text = "Продолжить";

	rxStatus = OK;
	auto commands = CyclogrammTextBox->Text->Split(gcnew array<String^>{"\r\n"}, StringSplitOptions::RemoveEmptyEntries);
	
	StepCount = System::Convert::ToInt32(stepCountNUD->Value) - 1;

	CyclogrammProgressBar->Maximum = commands->Length;
	CyclogrammProgressBar->BarColor = System::Drawing::Color::Green;
	CyclogrammProgressBar->Value = StepCount;

	progressStatusBox->Text = String::Format(gcnew String("{0}/{1}\r\n"),
		System::Convert::ToString(StepCount),
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
	CyclogrammProgressBar->BarColor = System::Drawing::Color::Red;
	PauseButton->Enabled = false;

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

	for each (String ^ elem in FocusCommandListBox->Items)
		commands->Add(elem + "()");
	for each (String ^ elem in MegaFrameCommandListBox->Items)
		commands->Add(elem + "()");
	for each (String ^ elem in HexapodCommandListBox->Items)
		commands->Add(elem + "()");

	popupMenu = gcnew FastColoredTextBoxNS::AutocompleteMenu(CyclogrammTextBox);
	popupMenu->Items->Width = 200;
	popupMenu->Items->SetAutocompleteItems(commands);

	FocusClient = gcnew SimpleTcpClient(FocusIpPortTB->Text);
	FocusClient->Events->Connected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnFocusConnected);
	FocusClient->Events->Disconnected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnFocusDisconnected);
	FocusClient->Events->DataReceived += gcnew System::EventHandler<SuperSimpleTcp::DataReceivedEventArgs^>(this, &Wagner::WagnerForm::OnFocusDataReceived);

	MegaFrameClient = gcnew SimpleTcpClient(MegaFrameIpPortTB->Text);
	MegaFrameClient->Events->Connected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnMegaFrameConnected);
	MegaFrameClient->Events->Disconnected += gcnew System::EventHandler<SuperSimpleTcp::ConnectionEventArgs^>(this, &Wagner::WagnerForm::OnMegaFrameDisconnected);
	MegaFrameClient->Events->DataReceived += gcnew System::EventHandler<SuperSimpleTcp::DataReceivedEventArgs^>(this, &Wagner::WagnerForm::OnMegaFrameDataReceived);

	HexapodClient = gcnew UdpClient(HexapodIpTB->Text, Convert::ToInt64(HexapodPortTB->Text));

	commandNames->Add("park", PARK);
	commandNames->Add("getPosition", GET_POSITION);
	commandNames->Add("moveTo", MOVE_TO);
	commandNames->Add("getErrors", GET_ERRORS);
	commandNames->Add("resetErrors", RESET_ERRORS);
	commandNames->Add("moveStep", MOVE_STEP);

	commandNames->Add("setExposureTime", SET_EXPOSURE_TIME);
	commandNames->Add("saveFrame", SAVE_FRAME);
	commandNames->Add("takeFrame", TAKE_FRAME);
	commandNames->Add("givePosition", GIVE_POSITION);
	commandNames->Add("pickUpExposure", PICK_UP_EXPOSURE);
	commandNames->Add("getExposureTime", GET_EXPOSURE_TIME);
	commandNames->Add("filterFrame", FILTER_FRAME);
	commandNames->Add("localizeFrame", LOCALIZE_FRAME);
}

System::Void Wagner::WagnerForm::FocusCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Text += FocusCommandListBox->SelectedItem->ToString() + "()" + "\r\n";
}

System::Void Wagner::WagnerForm::MegaFrameCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Text += MegaFrameCommandListBox->SelectedItem->ToString() + "()" + "\r\n";
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

System::Void Wagner::WagnerForm::cnctToMegaFrame_Click(System::Object^ sender, System::EventArgs^ e) {
	if (MegaFrameClient->IsConnected)
		MegaFrameClient->Disconnect();
	else {
		try {
			MegaFrameClient->Connect();
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
	String^ FocusFuncs = "^getPosition[(][)]|^park[(][)]|^getErrors[(][)]|^resetErrors[(][)]|moveStep[(][0-9]+[.]?[0-9]*[)]|moveTo[(][0-9]+[.]?[0-9]*[)]";
	String^ MegaFrameFuncs = "|^setExposureTime[(][0-9]+[)]|^saveFrame[(][)]|^takeFrame[(][)]|^givePosition[(][)]|^pickUpExposure[(][)]|^getExposureTime[(][)]|^filterFrame[(][)]|^localizeFrame[(][)]";
	String^ pattern = FocusFuncs + MegaFrameFuncs;
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
	WagnerPacket^ txMsg = gcnew WagnerPacket();
	int size = Marshal::SizeOf(txMsg);
	fromBytes(e->Data, txMsg, size);
	
	if (rxStatus == ON_WAITING_MSG) {
		ParsePackets^ action = gcnew ParsePackets(this, &WagnerForm::ParsePacket);
		this->BeginInvoke(action, txMsg);
	}
}

void Wagner::WagnerForm::UpdateMegaFrameConnected() {
	chatTextBox->Text += "MegaFrame подключен\r\n";
	cnctToMegaFrame->ForeColor = System::Drawing::Color::Red;
	cnctToMegaFrame->Text = "Отключить MegaFrame";
}

void Wagner::WagnerForm::UpdateMegaFrameDisconnected() {
	chatTextBox->Text += "MegaFrame Отключен\r\n";
	cnctToMegaFrame->ForeColor = System::Drawing::Color::DarkGreen;
	cnctToMegaFrame->Text = "Подключить MegaFrame";
}

void Wagner::WagnerForm::OnMegaFrameConnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	UpdateAction^ action = gcnew UpdateAction(this, &Wagner::WagnerForm::UpdateMegaFrameConnected);
	this->Invoke(action);
}

void Wagner::WagnerForm::OnMegaFrameDisconnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e) {
	UpdateAction^ action = gcnew UpdateAction(this, &Wagner::WagnerForm::UpdateMegaFrameDisconnected);
	this->Invoke(action);
}

void Wagner::WagnerForm::OnMegaFrameDataReceived(System::Object^ sender, SuperSimpleTcp::DataReceivedEventArgs^ e) {
	return;
}

#pragma endregion

#pragma region Bytes_Array Conversion

void Wagner::WagnerForm::toBytes(WagnerPacket^ %packet, array<Byte>^ bytes, int size) {
	IntPtr ptr = Marshal::AllocHGlobal(size);
	Marshal::StructureToPtr(packet, ptr, true);
	Marshal::Copy(ptr, bytes, 0, size);
	Marshal::FreeHGlobal(ptr);
}

void Wagner::WagnerForm::fromBytes(array<Byte>^ bytes, WagnerPacket^ %packet, int size) {
	IntPtr ptr = Marshal::AllocHGlobal(size);
	Marshal::Copy(bytes, 0, ptr, size);
	packet = (WagnerPacket^)Marshal::PtrToStructure(ptr, packet->GetType());
	Marshal::FreeHGlobal(ptr);
}

#pragma endregion

#pragma region ValidateTextBox

void Wagner::WagnerForm::ParseStringName(String^ s, uint32_t* commandParsed, String^% appName) {
	int charLocation = s->IndexOf("(", StringComparison::Ordinal);
	if (charLocation > 0 && commandNames->ContainsKey(s->Substring(0, charLocation))) {
		String^ funcName = s->Substring(0, charLocation);
		*commandParsed = commandNames[funcName];
		if (FocusCommandListBox->Items->Contains(funcName))
			appName = "Focus";
		else if (MegaFrameCommandListBox->Items->Contains(funcName))
			appName = "MegaFrame";
		else if (HexapodCommandListBox->Items->Contains(funcName))
			appName = "Hexapod";
		else
			appName = String::Empty;
	}
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
		double numDouble;
		uint32_t numInt;

		if (temp->Contains(".")) {
			if (System::Double::TryParse(temp, numDouble)) {
				numInt = Convert::ToUInt32(numDouble * 1000);
				args->Add(numInt);
			}
		} 
		else {
			if (System::UInt32::TryParse(temp, numInt))
				args->Add(numInt * 1000);
		}
	}
	return args;
}

bool Wagner::WagnerForm::isFunctionValid(String^ s) {
	String^ FocusFuncs = "^getPosition[(][)]|^park[(][)]|^getErrors[(][)]|^resetErrors[(][)]|moveStep[(][0-9]+[.]?[0-9]*[)]|moveTo[(][0-9]+[.]?[0-9]*[)]";
	String^ MegaFrameFuncs = "|^setExposureTime[(][0-9]+[)]|^saveFrame[(][)]|^takeFrame[(][)]|^givePosition[(][)]|^pickUpExposure[(][)]|^getExposureTime[(][)]|^filterFrame[(][)]|^localizeFrame[(][)]";
	String^ pattern = FocusFuncs + MegaFrameFuncs;

	Regex^ regex = gcnew Regex(pattern);
	Match^ match = regex->Match(s);
	if (match->Success)
		return true;
	return false;
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

#pragma region FocusFuncs

bool Wagner::WagnerForm::SendWagnerPacket(uint32_t cmd, uint8_t status_code, uint32_t data) {
	WagnerPacket^ packet = gcnew WagnerPacket();
	packet->command = cmd;
	packet->status_code = SUCCESS;
	packet->data = data;

	int size = Marshal::SizeOf(packet);
	array<Byte>^ msg = gcnew array<Byte>(size);
	toBytes(packet, msg, size);

	if (!FocusClient->IsConnected)
		return false;

	rxStatus = ON_WAITING_MSG;
	FocusClient->Send(msg);
	return true;
}

bool Wagner::WagnerForm::ParseWagnerPacket(WagnerPacket^ packet)	 {
	if (packet->status_code == SUCCESS) {
		float position = 0;
		switch(packet->command) {
		case GET_POSITION:
			position = ((float)packet->data) / 1000.0;
			chatTextBox->Text += String::Format(gcnew String("Текущая позиция: {0}\r\n"), System::Convert::ToString(position));
			waitMessage->Set();
			break;
		case MOVE_TO:
			position = ((float)packet->data) / 1000.0;
			chatTextBox->Text += String::Format(gcnew String("Команда: \"Движение в точку\" выполнена. Текущая позиция: {0}\r\n"), 
				System::Convert::ToString(position));
			waitMessage->Set();
			break;
		case MOVE_STEP:
			position = ((float)packet->data) / 1000.0;
			chatTextBox->Text += String::Format(gcnew String("Команда: \"Движение с шагом\" выполнена. Текущая позиция: {0}\r\n"), 
				System::Convert::ToString(position));
			waitMessage->Set();
			break;
		case PARK:
			position = ((float)packet->data) / 1000.0;
			chatTextBox->Text += String::Format(gcnew String("Команда: \"Парковка\" выполнена. Текущая позиция: {0}\r\n"), 
				System::Convert::ToString(position));
			waitMessage->Set();
			break;
		case GET_ERRORS:
			chatTextBox->Text += String::Format(gcnew String("Ошибка позиционирования: {0}.\r\nОшибка направления: {1}.\r\nНет движения: {2}.\r\n"),
				System::Convert::ToString(packet->pos_err),
				System::Convert::ToString(packet->dir_err),
				System::Convert::ToString(packet->mov_err));
			waitMessage->Set();
			break;
		case RESET_ERRORS:
			chatTextBox->Text += "Ошибки сброшены.\r\n";
			waitMessage->Set();
			break;
		}
		rxStatus = OK;
		return true;
	} 
	else {
		isPause = true;
		switch (packet->status_code) {
		case APP_IS_BUSY:
			chatTextBox->Text += "Wagner не может получить доступ к FocusAPP, так как это приложение занято. Повторите попытку.\r\n";
			waitMessage->Set();
			break;
		case REF_POINT_NOT_CAPTURED:
			chatTextBox->Text += "Референтная метка не захвачена. Захватите реф.метку через FocusAPP и повторите попытку.\r\n";
			waitMessage->Set();
			break;
		case SENSOR_ERROR:
			chatTextBox->Text += "Ошибка датчика. Проверьте соединение и повторите попытку.\r\n";
			waitMessage->Set();
			break;
		case STOPPED_IN_FOCUSAPP:
			chatTextBox->Text += "Движение в точку остановлено через FocusAPP.\r\n";
			waitMessage->Set();
			break;
		case POSITIONING_ERROR:
			chatTextBox->Text += String::Format(gcnew String("Ошибка позиционирования. Осталось: {0}\r\n"), 
				System::Convert::ToString(packet->data));
			waitMessage->Set();
			break;
		case WRONG_DIRECTION:
			chatTextBox->Text += String::Format(gcnew String("Ошибка направления. Осталось: {0}\r\n"),
				System::Convert::ToString(packet->data));
			waitMessage->Set();
			break;
		case NOT_MOVING:
			chatTextBox->Text += String::Format(gcnew String("Нет движения. Осталось: {0}\r\n"),
				System::Convert::ToString(packet->data));
			waitMessage->Set();
			break;
		case UNKNOWN_COMMAND:
			chatTextBox->Text += "Неизвестная команда.\r\n";
			waitMessage->Set();
			break;
		case OUT_OF_RANGE:
			chatTextBox->Text += "Заданное значение за пределами шкалы.\r\n";
			waitMessage->Set();
			break;
		default:
			chatTextBox->Text += "Неверный формат пакета.\r\n";
			waitMessage->Set();
			break;
		}
		rxStatus = ERROR_STATUS_CODE;
		return false;
	}
}

bool Wagner::WagnerForm::SendPacket(uint32_t cmd, uint8_t status_code, uint32_t data) {
	return;
}

bool Wagner::WagnerForm::ParsePacket(Packet^ packet) {
	return;
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

	while (StepCount < commands->Length) {
		if (DoCyclogrammWorker->CancellationPending)
			return;
	
	connectionLost:
		if (isPause) {
			UpdateAction^ pause = gcnew UpdateAction(this, &Wagner::WagnerForm::OnPausedHandler);
			this->Invoke(pause);
			pauseEvent->WaitOne();
			continue;
		}

		try {
			uint32_t funcName;
			String^ app;
			ParseStringName(commands[StepCount], &funcName, app);
			auto args = getFunctionArgsFromString(commands[StepCount]);
			bool isConnected;

			if (app == "Focus")
				bool isConnected = SendWagnerPacket(funcName, SUCCESS, args->Count == 0 ? 0 : args[0]);
			else
				bool isConnected = SendPacket(funcName, SUCCESS, args->Count == 0 ? 0 : args[0]);

			if (!isConnected) {
				MessageBox::Show(String::Format(gcnew String("Нет соединения с {0}\n"), app),
					Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
				isPause = true;
				goto connectionLost;
			}

			waitMessage->WaitOne(600000);
			Update^ action;

			switch (rxStatus) {
			case ON_WAITING_MSG:
				rxStatus = OK;
				isPause = true;
				action = gcnew Update(this, &Wagner::WagnerForm::UpdateChatBox);
				this->Invoke(action, "Таймаут запроса\r\n");
				break;
			case CANCELED:
				action = gcnew Update(this, &Wagner::WagnerForm::UpdateChatBox);
				this->Invoke(action, "Запрос отменен\r\n");
			case ERROR_STATUS_CODE:
				isPause = true;
				break;
			case OK:
				StepCount++;
				DoCyclogrammWorker->ReportProgress(1, commands->Length);
				break;
			}
		}
		catch (Exception^ ex) {
			MessageBox::Show(ex->Message, Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
			break;
		}
	}
	DoCyclogrammWorker->ReportProgress(1, commands->Length);
}

System::Void Wagner::WagnerForm::DoCyclogrammWorker_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e) {
	CyclogrammProgressBar->Increment(1);
	int TotalStep = (int)e->UserState;
		progressStatusBox->Text = String::Format(gcnew String("{0}/{1}\r\n"), 
			System::Convert::ToString(StepCount),
			System::Convert::ToString(TotalStep));
}

System::Void Wagner::WagnerForm::DoCyclogrammWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	rxStatus = OK;
	isPause = false;
	
	StartButton->Enabled = true;
	stepCountNUD->Enabled = true;
	LoadScriptBtn->Enabled = true;

	StartButton->Text = "Начать";

	PauseButton->Enabled = false;
	StopButton->Enabled = false;

	FocusCommandListBox->Enabled = true;
	FocusCommandListBox->Enabled = true;
	FocusCommandListBox->Enabled = true;

	cnctToFocus->Enabled = true;
	cnctToMegaFrame->Enabled = true;
	cnctToHexapod->Enabled = true;

	CyclogrammTextBox->ReadOnly = false;
	ClearCyclogrammButton->Enabled = true;

}

#pragma endregion