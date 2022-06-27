#pragma once
#include <iostream>

enum Commands {
	WHO_ARE_YOU = 0,
	GET_POSITION,
	MOVE_TO,
	PARK,
	GET_ERRORS,
	RESET_ERRORS
};

namespace Wagner {
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Collections::Generic;
	using namespace System::Runtime::InteropServices;
	using namespace System::Threading;

	using namespace CavemanTcp;
	/// <summary>
	/// Summary for WagnerForm
	/// </summary>
	public ref class WagnerForm : public System::Windows::Forms::Form
	{	

	[StructLayout(LayoutKind::Sequential)]
	ref struct  WagnerPacket {
		uint8_t command;
		[MarshalAs(UnmanagedType::U4)]
		uint32_t data;
	};
	private: 
		bool isPause = false;
		bool isReading = false;
		int StepCount = 0;
		CancellationTokenSource^ CancelReading = gcnew CancellationTokenSource();

	public:		
		delegate void Update(String^ msg);
		bool isScriptValid = false;

		Dictionary<String^, Action<uint32_t>^>^ funcs = gcnew Dictionary<String^, Action<uint32_t>^>();
		Dictionary<String^, String^>^ clientsDictionary = gcnew Dictionary<String^, String^>();

		CavemanTcpServer^ server;

	public:
		WagnerForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~WagnerForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private: System::Windows::Forms::TextBox^ ServerAdrress;
	private: System::Windows::Forms::Button^ PauseButton;
	private: System::Windows::Forms::Button^ StopButton;
	private: System::Windows::Forms::Button^ StartButton;
	private: System::Windows::Forms::Button^ ExpandButton;
	private: System::Windows::Forms::ListBox^ clientsListBox;
	private: System::Windows::Forms::ListBox^ CommandListBox;
	private: System::Windows::Forms::TextBox^ CommandTB;
	private: System::Windows::Forms::TextBox^ chatTextBox;
	private: System::Windows::Forms::RichTextBox^ CyclogrammTextBox;
	private: System::Windows::Forms::Button^ ClearCyclogrammButton;
	private: ProgressBarSample::TextProgressBar^ CyclogrammProgressBar;
	private: System::ComponentModel::BackgroundWorker^ DoCyclogrammWorker;

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(WagnerForm::typeid));
			this->ServerAdrress = (gcnew System::Windows::Forms::TextBox());
			this->PauseButton = (gcnew System::Windows::Forms::Button());
			this->StopButton = (gcnew System::Windows::Forms::Button());
			this->StartButton = (gcnew System::Windows::Forms::Button());
			this->ExpandButton = (gcnew System::Windows::Forms::Button());
			this->clientsListBox = (gcnew System::Windows::Forms::ListBox());
			this->CommandListBox = (gcnew System::Windows::Forms::ListBox());
			this->CommandTB = (gcnew System::Windows::Forms::TextBox());
			this->chatTextBox = (gcnew System::Windows::Forms::TextBox());
			this->CyclogrammTextBox = (gcnew System::Windows::Forms::RichTextBox());
			this->ClearCyclogrammButton = (gcnew System::Windows::Forms::Button());
			this->CyclogrammProgressBar = (gcnew ProgressBarSample::TextProgressBar());
			this->DoCyclogrammWorker = (gcnew System::ComponentModel::BackgroundWorker());
			this->SuspendLayout();
			// 
			// ServerAdrress
			// 
			this->ServerAdrress->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->ServerAdrress->Location = System::Drawing::Point(243, 14);
			this->ServerAdrress->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->ServerAdrress->Multiline = true;
			this->ServerAdrress->Name = L"ServerAdrress";
			this->ServerAdrress->ReadOnly = true;
			this->ServerAdrress->Size = System::Drawing::Size(463, 26);
			this->ServerAdrress->TabIndex = 3;
			this->ServerAdrress->Text = L"127.0.0.1:9000";
			// 
			// PauseButton
			// 
			this->PauseButton->Enabled = false;
			this->PauseButton->Location = System::Drawing::Point(8, 48);
			this->PauseButton->Margin = System::Windows::Forms::Padding(4);
			this->PauseButton->Name = L"PauseButton";
			this->PauseButton->Size = System::Drawing::Size(100, 28);
			this->PauseButton->TabIndex = 4;
			this->PauseButton->Text = L"Пауза";
			this->PauseButton->UseVisualStyleBackColor = true;
			this->PauseButton->Click += gcnew System::EventHandler(this, &WagnerForm::PauseButton_Click);
			// 
			// StopButton
			// 
			this->StopButton->Enabled = false;
			this->StopButton->Location = System::Drawing::Point(8, 84);
			this->StopButton->Margin = System::Windows::Forms::Padding(4);
			this->StopButton->Name = L"StopButton";
			this->StopButton->Size = System::Drawing::Size(100, 28);
			this->StopButton->TabIndex = 5;
			this->StopButton->Text = L"Стоп";
			this->StopButton->UseVisualStyleBackColor = true;
			this->StopButton->Click += gcnew System::EventHandler(this, &WagnerForm::StopButton_Click);
			// 
			// StartButton
			// 
			this->StartButton->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->StartButton->Location = System::Drawing::Point(8, 12);
			this->StartButton->Margin = System::Windows::Forms::Padding(4);
			this->StartButton->Name = L"StartButton";
			this->StartButton->Size = System::Drawing::Size(100, 28);
			this->StartButton->TabIndex = 7;
			this->StartButton->Text = L"Начать";
			this->StartButton->UseVisualStyleBackColor = true;
			this->StartButton->Click += gcnew System::EventHandler(this, &WagnerForm::StartButton_Click);
			// 
			// ExpandButton
			// 
			this->ExpandButton->Location = System::Drawing::Point(713, 225);
			this->ExpandButton->Margin = System::Windows::Forms::Padding(4);
			this->ExpandButton->Name = L"ExpandButton";
			this->ExpandButton->Size = System::Drawing::Size(27, 207);
			this->ExpandButton->TabIndex = 8;
			this->ExpandButton->Text = L"<";
			this->ExpandButton->UseVisualStyleBackColor = true;
			this->ExpandButton->Click += gcnew System::EventHandler(this, &WagnerForm::ExpandButton_Click);
			// 
			// clientsListBox
			// 
			this->clientsListBox->FormattingEnabled = true;
			this->clientsListBox->ItemHeight = 16;
			this->clientsListBox->Location = System::Drawing::Point(116, 14);
			this->clientsListBox->Margin = System::Windows::Forms::Padding(4);
			this->clientsListBox->Name = L"clientsListBox";
			this->clientsListBox->Size = System::Drawing::Size(119, 244);
			this->clientsListBox->TabIndex = 9;
			// 
			// CommandListBox
			// 
			this->CommandListBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->CommandListBox->FormattingEnabled = true;
			this->CommandListBox->ItemHeight = 20;
			this->CommandListBox->Items->AddRange(gcnew cli::array< System::Object^  >(5) {
				L"getPosition", L"moveTo", L"park", L"getErrors",
					L"resetErrors"
			});
			this->CommandListBox->Location = System::Drawing::Point(748, 50);
			this->CommandListBox->Margin = System::Windows::Forms::Padding(4);
			this->CommandListBox->Name = L"CommandListBox";
			this->CommandListBox->Size = System::Drawing::Size(175, 604);
			this->CommandListBox->TabIndex = 10;
			this->CommandListBox->MouseDoubleClick += gcnew System::Windows::Forms::MouseEventHandler(this, &WagnerForm::CommandListBox_MouseDoubleClick);
			// 
			// CommandTB
			// 
			this->CommandTB->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->CommandTB->Location = System::Drawing::Point(748, 14);
			this->CommandTB->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->CommandTB->Multiline = true;
			this->CommandTB->Name = L"CommandTB";
			this->CommandTB->ReadOnly = true;
			this->CommandTB->Size = System::Drawing::Size(175, 26);
			this->CommandTB->TabIndex = 11;
			this->CommandTB->Text = L"Команды";
			this->CommandTB->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// chatTextBox
			// 
			this->chatTextBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->chatTextBox->Location = System::Drawing::Point(243, 48);
			this->chatTextBox->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->chatTextBox->Multiline = true;
			this->chatTextBox->Name = L"chatTextBox";
			this->chatTextBox->ReadOnly = true;
			this->chatTextBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->chatTextBox->Size = System::Drawing::Size(463, 210);
			this->chatTextBox->TabIndex = 12;
			// 
			// CyclogrammTextBox
			// 
			this->CyclogrammTextBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->CyclogrammTextBox->Location = System::Drawing::Point(8, 302);
			this->CyclogrammTextBox->Margin = System::Windows::Forms::Padding(4);
			this->CyclogrammTextBox->Name = L"CyclogrammTextBox";
			this->CyclogrammTextBox->Size = System::Drawing::Size(699, 379);
			this->CyclogrammTextBox->TabIndex = 14;
			this->CyclogrammTextBox->Text = L" ";
			this->CyclogrammTextBox->Leave += gcnew System::EventHandler(this, &WagnerForm::CyclogrammTextBox_Leave);
			// 
			// ClearCyclogrammButton
			// 
			this->ClearCyclogrammButton->Location = System::Drawing::Point(8, 230);
			this->ClearCyclogrammButton->Margin = System::Windows::Forms::Padding(4);
			this->ClearCyclogrammButton->Name = L"ClearCyclogrammButton";
			this->ClearCyclogrammButton->Size = System::Drawing::Size(100, 28);
			this->ClearCyclogrammButton->TabIndex = 15;
			this->ClearCyclogrammButton->Text = L"Очистить";
			this->ClearCyclogrammButton->UseVisualStyleBackColor = true;
			this->ClearCyclogrammButton->Click += gcnew System::EventHandler(this, &WagnerForm::ClearCyclogrammButton_Click);
			// 
			// CyclogrammProgressBar
			// 
			this->CyclogrammProgressBar->CustomText = L"";
			this->CyclogrammProgressBar->Location = System::Drawing::Point(8, 265);
			this->CyclogrammProgressBar->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
			this->CyclogrammProgressBar->Name = L"CyclogrammProgressBar";
			this->CyclogrammProgressBar->ProgressColor = System::Drawing::Color::LightGreen;
			this->CyclogrammProgressBar->Size = System::Drawing::Size(699, 30);
			this->CyclogrammProgressBar->TabIndex = 16;
			this->CyclogrammProgressBar->TextColor = System::Drawing::Color::Black;
			this->CyclogrammProgressBar->TextFont = (gcnew System::Drawing::Font(L"Times New Roman", 11, System::Drawing::FontStyle::Bold));
			this->CyclogrammProgressBar->VisualMode = ProgressBarSample::ProgressBarDisplayMode::CurrProgress;
			// 
			// DoCyclogrammWorker
			// 
			this->DoCyclogrammWorker->WorkerReportsProgress = true;
			this->DoCyclogrammWorker->WorkerSupportsCancellation = true;
			this->DoCyclogrammWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &WagnerForm::DoCyclogrammWorker_DoWork);
			this->DoCyclogrammWorker->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &WagnerForm::DoCyclogrammWorker_ProgressChanged);
			// 
			// WagnerForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(939, 686);
			this->Controls->Add(this->CyclogrammProgressBar);
			this->Controls->Add(this->ClearCyclogrammButton);
			this->Controls->Add(this->CyclogrammTextBox);
			this->Controls->Add(this->chatTextBox);
			this->Controls->Add(this->CommandTB);
			this->Controls->Add(this->CommandListBox);
			this->Controls->Add(this->clientsListBox);
			this->Controls->Add(this->ExpandButton);
			this->Controls->Add(this->StartButton);
			this->Controls->Add(this->StopButton);
			this->Controls->Add(this->PauseButton);
			this->Controls->Add(this->ServerAdrress);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Margin = System::Windows::Forms::Padding(4);
			this->MaximizeBox = false;
			this->Name = L"WagnerForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Wagner";
			this->Load += gcnew System::EventHandler(this, &WagnerForm::WagnerForm_Load);
			this->ResumeLayout(false);
			this->PerformLayout();

		}

#pragma endregion

private: System::Void ExpandButton_Click(System::Object^ sender, System::EventArgs^ e);

private: System::Void StartButton_Click(System::Object^ sender, System::EventArgs^ e);

private: System::Void PauseButton_Click(System::Object^ sender, System::EventArgs^ e);

private: System::Void StopButton_Click(System::Object^ sender, System::EventArgs^ e);

private: System::Void WagnerForm_Load(System::Object^ sender, System::EventArgs^ e);

private: System::Void CommandListBox_MouseDoubleClick(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e);

private: System::Void ClearCyclogrammButton_Click(System::Object^ sender, System::EventArgs^ e);

	   void OnDragDrop(System::Object^ sender, System::Windows::Forms::DragEventArgs^ e);

#pragma region Server

	   void UpdateChatBox(String^ text);
	   void UpdateClientConnected(String^ ip);
	   void UpdateClientDisconnected(String^ ip);

	   void OnClientConnected(System::Object^ sender, CavemanTcp::ClientConnectedEventArgs^ e);
	   void OnClientDisconnected(System::Object^ sender, CavemanTcp::ClientDisconnectedEventArgs^ e);

#pragma endregion

#pragma region MarshallingPackets

	   array<Byte>^ getBytes(WagnerPacket^ packet);

	   WagnerPacket^ fromBytes(array<Byte>^ arr);

#pragma endregion

#pragma region ValidateTextBox

private: System::Void CyclogrammTextBox_Leave(System::Object^ sender, System::EventArgs^ e);

	   String^ getFunctionFromString(String^ s);

	   List<uint32_t>^ getArgsFromString(String^ s);

	   bool functionParser(String^ s);

	   void ValidateText();

#pragma endregion

#pragma region Funcs

	   void GetPosition(uint32_t data);

	   void MoveTo(uint32_t data);

	   void Park(uint32_t data);

	   void GetErrors(uint32_t data);

	   void ResetErrors(uint32_t data);

#pragma endregion

#pragma region Cyclogramm

private: System::Void DoCyclogrammWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);

private: System::Void DoCyclogrammWorker_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e);

#pragma endregion

};
}
