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
	return;
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

}

System::Void Wagner::WagnerForm::CommandListBox_MouseDoubleClick(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
	CyclogrammTextBox->Text += CommandListBox->SelectedItem->ToString() + "()" + "\r\n";
}

void Wagner::WagnerForm::OnDragDrop(System::Object^ sender, System::Windows::Forms::DragEventArgs^ e) {
	array<String^>^ paths = safe_cast<array<String^>^>(e->Data->GetData(DataFormats::FileDrop));
	for each (String ^ path in paths) {
		String^ ext = System::IO::Path::GetExtension(path)->ToLower();
		if (ext == ".txt") CyclogrammTextBox->AppendText(System::IO::File::ReadAllText(path));
	}
}

System::Void Wagner::WagnerForm::ClearCyclogrammButton_Click(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammTextBox->Clear();
}

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


void Wagner::WagnerForm::doFunction() {
	WagnerPacket^ packet1 = gcnew WagnerPacket();
	packet1->command = GET_ERRORS;
	packet1->data = 0;
	
	auto message = getBytes(packet1);
	server->Send(clientsListBox->SelectedItem->ToString(), message);
}

