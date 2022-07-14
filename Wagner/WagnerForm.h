#pragma once
#include <iostream>

enum ReadStatus {
	INITIAL_STATUS,
	ON_WAITING_MSG,
	CANCELED,
	READING_PACKET,
};

enum Commands {
	WHO_ARE_YOU = 10,
	GET_POSITION = 20,
	MOVE_TO = 30,
	GET_ERRORS = 40,
	RESET_ERRORS = 50
};

enum ErrorCode {
	APP_IS_BUSY = 0x18704,
	REF_POINT_NOT_CAPTURED,
	SENSOR_ERROR,
	STOPPED_IN_FOCUSAPP,
	POSITIONING_ERROR,
	WRONG_DIRECTION,
	NOT_MOVING,
	UNKNOWN_COMMAND
};

enum Apps {
	FOCUS = 0xD0,
	DATAFRAME = 0xDE,
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
	using namespace System::Net;
	using namespace System::Net::Sockets;
	using namespace SuperSimpleTcp;

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
		ReadStatus rxStatus;
		int StepCount = 1;

		AutoResetEvent^ pauseEvent = gcnew AutoResetEvent(false);
		AutoResetEvent^ waitMessage = gcnew AutoResetEvent(false);

	public:
		delegate bool ExecuteCommand(uint32_t data);
		delegate bool ParsePackets(WagnerPacket^ packet);
		delegate void Update(String^ msg);
		delegate void UpdateAction();
		bool isScriptValid = false;

		SimpleTcpClient^ FocusClient;
		SimpleTcpClient^ DataFrameClient;
		UdpClient^ HexapodClient;

		Dictionary<String^, ExecuteCommand^>^ funcs = gcnew Dictionary<String^, ExecuteCommand^>();
		Dictionary<String^, SimpleTcpClient^>^ connections = gcnew Dictionary<String^, SimpleTcpClient^>();

		List<String^>^ FocusFuncs = gcnew List<String^>();
		List<String^>^ DataFrameFuncs = gcnew List<String^>();
	private: System::Windows::Forms::TabControl^ tabControl1;
	public:
	private: System::Windows::Forms::TabPage^ ScriptPage;
	private: System::Windows::Forms::TabPage^ TgBot;

	private: System::Windows::Forms::TextBox^ progressStatusBox;


	private: System::Windows::Forms::Label^ StatusLabel;
	private: System::Windows::Forms::Button^ StartButton;
	private: System::Windows::Forms::Button^ StartFromBtn;

	private: System::Windows::Forms::Button^ LoadScriptBtn;


	private: System::Windows::Forms::Button^ SaveScriptBtn;
	private: System::Windows::Forms::NumericUpDown^ stepCountNUD;

	private: System::Windows::Forms::SaveFileDialog^ saveScriptFile;
	private: System::Windows::Forms::OpenFileDialog^ openScriptFile;
	private: ::ColorProgressBar::ColorProgressBar^ CyclogrammProgressBar;
		   List<String^>^ HexapodFuncs = gcnew List<String^>();

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
	private: System::Windows::Forms::ListBox^ DataFrameCommandListBox;
	private: System::Windows::Forms::Button^ PauseButton;
	private: System::Windows::Forms::Button^ StopButton;

	private: System::Windows::Forms::Button^ ExpandButton;
	private: System::Windows::Forms::ListBox^ FocusCommandListBox;
	private: System::Windows::Forms::Button^ cnctToDataFrame;
	private: System::Windows::Forms::Button^ cnctToFocus;
	private: System::Windows::Forms::Button^ cnctToHexapod;
	private: System::Windows::Forms::ListBox^ HexapodCommandListBox;
	private: System::Windows::Forms::TextBox^ FocusIpPortTB;
	private: System::Windows::Forms::TextBox^ DataFrameIpPortTB;
	private: System::Windows::Forms::TextBox^ HexapodIpTB;
	private: System::Windows::Forms::TextBox^ HexapodPortTB;
	private: System::Windows::Forms::TextBox^ CommandTB;
	private: System::Windows::Forms::TextBox^ chatTextBox;
	private: System::Windows::Forms::RichTextBox^ CyclogrammTextBox;
	private: System::Windows::Forms::Button^ ClearCyclogrammButton;

	private: System::ComponentModel::BackgroundWorker^ DoCyclogrammWorker;

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(WagnerForm::typeid));
			this->PauseButton = (gcnew System::Windows::Forms::Button());
			this->StopButton = (gcnew System::Windows::Forms::Button());
			this->ExpandButton = (gcnew System::Windows::Forms::Button());
			this->FocusCommandListBox = (gcnew System::Windows::Forms::ListBox());
			this->CommandTB = (gcnew System::Windows::Forms::TextBox());
			this->chatTextBox = (gcnew System::Windows::Forms::TextBox());
			this->CyclogrammTextBox = (gcnew System::Windows::Forms::RichTextBox());
			this->ClearCyclogrammButton = (gcnew System::Windows::Forms::Button());
			this->DoCyclogrammWorker = (gcnew System::ComponentModel::BackgroundWorker());
			this->cnctToDataFrame = (gcnew System::Windows::Forms::Button());
			this->cnctToFocus = (gcnew System::Windows::Forms::Button());
			this->cnctToHexapod = (gcnew System::Windows::Forms::Button());
			this->DataFrameCommandListBox = (gcnew System::Windows::Forms::ListBox());
			this->HexapodCommandListBox = (gcnew System::Windows::Forms::ListBox());
			this->FocusIpPortTB = (gcnew System::Windows::Forms::TextBox());
			this->DataFrameIpPortTB = (gcnew System::Windows::Forms::TextBox());
			this->HexapodIpTB = (gcnew System::Windows::Forms::TextBox());
			this->HexapodPortTB = (gcnew System::Windows::Forms::TextBox());
			this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
			this->ScriptPage = (gcnew System::Windows::Forms::TabPage());
			this->stepCountNUD = (gcnew System::Windows::Forms::NumericUpDown());
			this->StartFromBtn = (gcnew System::Windows::Forms::Button());
			this->LoadScriptBtn = (gcnew System::Windows::Forms::Button());
			this->SaveScriptBtn = (gcnew System::Windows::Forms::Button());
			this->progressStatusBox = (gcnew System::Windows::Forms::TextBox());
			this->StatusLabel = (gcnew System::Windows::Forms::Label());
			this->StartButton = (gcnew System::Windows::Forms::Button());
			this->TgBot = (gcnew System::Windows::Forms::TabPage());
			this->saveScriptFile = (gcnew System::Windows::Forms::SaveFileDialog());
			this->openScriptFile = (gcnew System::Windows::Forms::OpenFileDialog());
			this->CyclogrammProgressBar = (gcnew ::ColorProgressBar::ColorProgressBar());
			this->tabControl1->SuspendLayout();
			this->ScriptPage->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->stepCountNUD))->BeginInit();
			this->SuspendLayout();
			// 
			// PauseButton
			// 
			this->PauseButton->Enabled = false;
			this->PauseButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->PauseButton->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"PauseButton.Image")));
			this->PauseButton->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->PauseButton->Location = System::Drawing::Point(95, 400);
			this->PauseButton->Name = L"PauseButton";
			this->PauseButton->Size = System::Drawing::Size(77, 29);
			this->PauseButton->TabIndex = 4;
			this->PauseButton->Text = L"Пауза";
			this->PauseButton->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->PauseButton->UseVisualStyleBackColor = true;
			this->PauseButton->Click += gcnew System::EventHandler(this, &WagnerForm::PauseButton_Click);
			// 
			// StopButton
			// 
			this->StopButton->Enabled = false;
			this->StopButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->StopButton->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"StopButton.Image")));
			this->StopButton->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->StopButton->Location = System::Drawing::Point(178, 400);
			this->StopButton->Name = L"StopButton";
			this->StopButton->Size = System::Drawing::Size(67, 29);
			this->StopButton->TabIndex = 5;
			this->StopButton->Text = L"Стоп";
			this->StopButton->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->StopButton->UseVisualStyleBackColor = true;
			this->StopButton->Click += gcnew System::EventHandler(this, &WagnerForm::StopButton_Click);
			// 
			// ExpandButton
			// 
			this->ExpandButton->Location = System::Drawing::Point(660, 184);
			this->ExpandButton->Name = L"ExpandButton";
			this->ExpandButton->Size = System::Drawing::Size(20, 168);
			this->ExpandButton->TabIndex = 8;
			this->ExpandButton->Text = L">";
			this->ExpandButton->UseVisualStyleBackColor = true;
			this->ExpandButton->Click += gcnew System::EventHandler(this, &WagnerForm::ExpandButton_Click);
			// 
			// FocusCommandListBox
			// 
			this->FocusCommandListBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->FocusCommandListBox->FormattingEnabled = true;
			this->FocusCommandListBox->ItemHeight = 16;
			this->FocusCommandListBox->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"getPosition", L"moveTo", L"park", L"getErrors",
					L"resetErrors", L"moveStep"
			});
			this->FocusCommandListBox->Location = System::Drawing::Point(686, 42);
			this->FocusCommandListBox->Name = L"FocusCommandListBox";
			this->FocusCommandListBox->Size = System::Drawing::Size(132, 148);
			this->FocusCommandListBox->TabIndex = 10;
			this->FocusCommandListBox->SelectedIndexChanged += gcnew System::EventHandler(this, &WagnerForm::FocusCommandListBox_SelectedIndexChanged);
			// 
			// CommandTB
			// 
			this->CommandTB->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->CommandTB->Location = System::Drawing::Point(686, 12);
			this->CommandTB->Margin = System::Windows::Forms::Padding(2);
			this->CommandTB->Multiline = true;
			this->CommandTB->Name = L"CommandTB";
			this->CommandTB->ReadOnly = true;
			this->CommandTB->Size = System::Drawing::Size(132, 22);
			this->CommandTB->TabIndex = 11;
			this->CommandTB->Text = L"Команды";
			this->CommandTB->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// chatTextBox
			// 
			this->chatTextBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->chatTextBox->Location = System::Drawing::Point(12, 476);
			this->chatTextBox->Margin = System::Windows::Forms::Padding(2);
			this->chatTextBox->Multiline = true;
			this->chatTextBox->Name = L"chatTextBox";
			this->chatTextBox->ReadOnly = true;
			this->chatTextBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->chatTextBox->Size = System::Drawing::Size(643, 234);
			this->chatTextBox->TabIndex = 12;
			// 
			// CyclogrammTextBox
			// 
			this->CyclogrammTextBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->CyclogrammTextBox->Location = System::Drawing::Point(6, 92);
			this->CyclogrammTextBox->Name = L"CyclogrammTextBox";
			this->CyclogrammTextBox->Size = System::Drawing::Size(623, 302);
			this->CyclogrammTextBox->TabIndex = 14;
			this->CyclogrammTextBox->Text = L" ";
			this->CyclogrammTextBox->Leave += gcnew System::EventHandler(this, &WagnerForm::CyclogrammTextBox_Leave);
			// 
			// ClearCyclogrammButton
			// 
			this->ClearCyclogrammButton->Location = System::Drawing::Point(497, -1);
			this->ClearCyclogrammButton->Name = L"ClearCyclogrammButton";
			this->ClearCyclogrammButton->Size = System::Drawing::Size(78, 29);
			this->ClearCyclogrammButton->TabIndex = 15;
			this->ClearCyclogrammButton->Text = L"Очистить";
			this->ClearCyclogrammButton->UseVisualStyleBackColor = true;
			this->ClearCyclogrammButton->Click += gcnew System::EventHandler(this, &WagnerForm::ClearCyclogrammButton_Click);
			// 
			// DoCyclogrammWorker
			// 
			this->DoCyclogrammWorker->WorkerReportsProgress = true;
			this->DoCyclogrammWorker->WorkerSupportsCancellation = true;
			this->DoCyclogrammWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &WagnerForm::DoCyclogrammWorker_DoWork);
			this->DoCyclogrammWorker->ProgressChanged += gcnew System::ComponentModel::ProgressChangedEventHandler(this, &WagnerForm::DoCyclogrammWorker_ProgressChanged);
			this->DoCyclogrammWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &WagnerForm::DoCyclogrammWorker_RunWorkerCompleted);
			// 
			// cnctToDataFrame
			// 
			this->cnctToDataFrame->ForeColor = System::Drawing::Color::DarkGreen;
			this->cnctToDataFrame->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->cnctToDataFrame->Location = System::Drawing::Point(168, 33);
			this->cnctToDataFrame->Name = L"cnctToDataFrame";
			this->cnctToDataFrame->Size = System::Drawing::Size(156, 23);
			this->cnctToDataFrame->TabIndex = 17;
			this->cnctToDataFrame->Text = L"Подключить DataFrame";
			this->cnctToDataFrame->UseVisualStyleBackColor = true;
			this->cnctToDataFrame->Click += gcnew System::EventHandler(this, &WagnerForm::cnctToDataFrame_Click);
			// 
			// cnctToFocus
			// 
			this->cnctToFocus->ForeColor = System::Drawing::Color::DarkGreen;
			this->cnctToFocus->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->cnctToFocus->Location = System::Drawing::Point(6, 33);
			this->cnctToFocus->Name = L"cnctToFocus";
			this->cnctToFocus->Size = System::Drawing::Size(156, 23);
			this->cnctToFocus->TabIndex = 18;
			this->cnctToFocus->Text = L"Подключить Focus";
			this->cnctToFocus->UseVisualStyleBackColor = true;
			this->cnctToFocus->Click += gcnew System::EventHandler(this, &WagnerForm::cnctToFocus_Click);
			// 
			// cnctToHexapod
			// 
			this->cnctToHexapod->ForeColor = System::Drawing::Color::DarkGreen;
			this->cnctToHexapod->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->cnctToHexapod->Location = System::Drawing::Point(330, 33);
			this->cnctToHexapod->Name = L"cnctToHexapod";
			this->cnctToHexapod->Size = System::Drawing::Size(156, 23);
			this->cnctToHexapod->TabIndex = 19;
			this->cnctToHexapod->Text = L"Подключить Hexapod";
			this->cnctToHexapod->UseVisualStyleBackColor = true;
			this->cnctToHexapod->Click += gcnew System::EventHandler(this, &WagnerForm::cnctToHexapod_Click);
			// 
			// DataFrameCommandListBox
			// 
			this->DataFrameCommandListBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->DataFrameCommandListBox->FormattingEnabled = true;
			this->DataFrameCommandListBox->ItemHeight = 16;
			this->DataFrameCommandListBox->Location = System::Drawing::Point(686, 216);
			this->DataFrameCommandListBox->Name = L"DataFrameCommandListBox";
			this->DataFrameCommandListBox->Size = System::Drawing::Size(132, 148);
			this->DataFrameCommandListBox->TabIndex = 20;
			this->DataFrameCommandListBox->SelectedIndexChanged += gcnew System::EventHandler(this, &WagnerForm::DataFrameCommandListBox_SelectedIndexChanged);
			// 
			// HexapodCommandListBox
			// 
			this->HexapodCommandListBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->HexapodCommandListBox->FormattingEnabled = true;
			this->HexapodCommandListBox->ItemHeight = 16;
			this->HexapodCommandListBox->Items->AddRange(gcnew cli::array< System::Object^  >(4) {
				L"linearMove", L"angularMove", L"combinedMove",
					L"moveToZero"
			});
			this->HexapodCommandListBox->Location = System::Drawing::Point(685, 391);
			this->HexapodCommandListBox->Name = L"HexapodCommandListBox";
			this->HexapodCommandListBox->Size = System::Drawing::Size(132, 148);
			this->HexapodCommandListBox->TabIndex = 21;
			this->HexapodCommandListBox->SelectedIndexChanged += gcnew System::EventHandler(this, &WagnerForm::HexapodCommandListBox_SelectedIndexChanged);
			// 
			// FocusIpPortTB
			// 
			this->FocusIpPortTB->Location = System::Drawing::Point(6, 7);
			this->FocusIpPortTB->Name = L"FocusIpPortTB";
			this->FocusIpPortTB->ReadOnly = true;
			this->FocusIpPortTB->Size = System::Drawing::Size(156, 20);
			this->FocusIpPortTB->TabIndex = 22;
			this->FocusIpPortTB->Text = L"127.0.0.1:9000";
			// 
			// DataFrameIpPortTB
			// 
			this->DataFrameIpPortTB->Location = System::Drawing::Point(168, 7);
			this->DataFrameIpPortTB->Name = L"DataFrameIpPortTB";
			this->DataFrameIpPortTB->ReadOnly = true;
			this->DataFrameIpPortTB->Size = System::Drawing::Size(156, 20);
			this->DataFrameIpPortTB->TabIndex = 23;
			this->DataFrameIpPortTB->Text = L"127.0.0.1:6000";
			// 
			// HexapodIpTB
			// 
			this->HexapodIpTB->Location = System::Drawing::Point(330, 7);
			this->HexapodIpTB->Name = L"HexapodIpTB";
			this->HexapodIpTB->ReadOnly = true;
			this->HexapodIpTB->Size = System::Drawing::Size(96, 20);
			this->HexapodIpTB->TabIndex = 24;
			this->HexapodIpTB->Text = L"192.168.1.10";
			// 
			// HexapodPortTB
			// 
			this->HexapodPortTB->Location = System::Drawing::Point(432, 7);
			this->HexapodPortTB->Name = L"HexapodPortTB";
			this->HexapodPortTB->ReadOnly = true;
			this->HexapodPortTB->Size = System::Drawing::Size(54, 20);
			this->HexapodPortTB->TabIndex = 25;
			this->HexapodPortTB->Text = L"6000";
			// 
			// tabControl1
			// 
			this->tabControl1->Controls->Add(this->ScriptPage);
			this->tabControl1->Controls->Add(this->TgBot);
			this->tabControl1->Location = System::Drawing::Point(12, 12);
			this->tabControl1->Name = L"tabControl1";
			this->tabControl1->SelectedIndex = 0;
			this->tabControl1->Size = System::Drawing::Size(643, 463);
			this->tabControl1->TabIndex = 26;
			// 
			// ScriptPage
			// 
			this->ScriptPage->BackColor = System::Drawing::SystemColors::Control;
			this->ScriptPage->Controls->Add(this->CyclogrammProgressBar);
			this->ScriptPage->Controls->Add(this->stepCountNUD);
			this->ScriptPage->Controls->Add(this->StartFromBtn);
			this->ScriptPage->Controls->Add(this->LoadScriptBtn);
			this->ScriptPage->Controls->Add(this->SaveScriptBtn);
			this->ScriptPage->Controls->Add(this->progressStatusBox);
			this->ScriptPage->Controls->Add(this->FocusIpPortTB);
			this->ScriptPage->Controls->Add(this->StatusLabel);
			this->ScriptPage->Controls->Add(this->HexapodPortTB);
			this->ScriptPage->Controls->Add(this->cnctToFocus);
			this->ScriptPage->Controls->Add(this->HexapodIpTB);
			this->ScriptPage->Controls->Add(this->DataFrameIpPortTB);
			this->ScriptPage->Controls->Add(this->cnctToHexapod);
			this->ScriptPage->Controls->Add(this->PauseButton);
			this->ScriptPage->Controls->Add(this->StartButton);
			this->ScriptPage->Controls->Add(this->StopButton);
			this->ScriptPage->Controls->Add(this->CyclogrammTextBox);
			this->ScriptPage->Controls->Add(this->cnctToDataFrame);
			this->ScriptPage->Location = System::Drawing::Point(4, 22);
			this->ScriptPage->Name = L"ScriptPage";
			this->ScriptPage->Padding = System::Windows::Forms::Padding(3);
			this->ScriptPage->Size = System::Drawing::Size(635, 437);
			this->ScriptPage->TabIndex = 0;
			this->ScriptPage->Text = L"Скрипт";
			// 
			// stepCountNUD
			// 
			this->stepCountNUD->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->stepCountNUD->Location = System::Drawing::Point(345, 403);
			this->stepCountNUD->Name = L"stepCountNUD";
			this->stepCountNUD->Size = System::Drawing::Size(50, 24);
			this->stepCountNUD->TabIndex = 42;
			// 
			// StartFromBtn
			// 
			this->StartFromBtn->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->StartFromBtn->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"StartFromBtn.Image")));
			this->StartFromBtn->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->StartFromBtn->Location = System::Drawing::Point(251, 400);
			this->StartFromBtn->Name = L"StartFromBtn";
			this->StartFromBtn->Size = System::Drawing::Size(88, 29);
			this->StartFromBtn->TabIndex = 41;
			this->StartFromBtn->Text = L"Начать c";
			this->StartFromBtn->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->StartFromBtn->UseVisualStyleBackColor = true;
			// 
			// LoadScriptBtn
			// 
			this->LoadScriptBtn->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->LoadScriptBtn->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"LoadScriptBtn.Image")));
			this->LoadScriptBtn->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->LoadScriptBtn->Location = System::Drawing::Point(534, 400);
			this->LoadScriptBtn->Name = L"LoadScriptBtn";
			this->LoadScriptBtn->Size = System::Drawing::Size(95, 29);
			this->LoadScriptBtn->TabIndex = 40;
			this->LoadScriptBtn->Text = L"Загрузить";
			this->LoadScriptBtn->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->LoadScriptBtn->UseVisualStyleBackColor = true;
			this->LoadScriptBtn->Click += gcnew System::EventHandler(this, &WagnerForm::LoadScriptBtn_Click);
			// 
			// SaveScriptBtn
			// 
			this->SaveScriptBtn->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->SaveScriptBtn->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"SaveScriptBtn.Image")));
			this->SaveScriptBtn->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->SaveScriptBtn->Location = System::Drawing::Point(432, 400);
			this->SaveScriptBtn->Name = L"SaveScriptBtn";
			this->SaveScriptBtn->Size = System::Drawing::Size(100, 29);
			this->SaveScriptBtn->TabIndex = 39;
			this->SaveScriptBtn->Text = L"Сохранить";
			this->SaveScriptBtn->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->SaveScriptBtn->UseVisualStyleBackColor = true;
			this->SaveScriptBtn->Click += gcnew System::EventHandler(this, &WagnerForm::SaveScriptBtn_Click);
			// 
			// progressStatusBox
			// 
			this->progressStatusBox->Font = (gcnew System::Drawing::Font(L"MS Gothic", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->progressStatusBox->Location = System::Drawing::Point(500, 22);
			this->progressStatusBox->Margin = System::Windows::Forms::Padding(2);
			this->progressStatusBox->Name = L"progressStatusBox";
			this->progressStatusBox->ReadOnly = true;
			this->progressStatusBox->Size = System::Drawing::Size(129, 20);
			this->progressStatusBox->TabIndex = 37;
			this->progressStatusBox->Text = L"0/0";
			this->progressStatusBox->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
			// 
			// StatusLabel
			// 
			this->StatusLabel->AutoSize = true;
			this->StatusLabel->ForeColor = System::Drawing::SystemColors::ControlText;
			this->StatusLabel->Location = System::Drawing::Point(497, 7);
			this->StatusLabel->Margin = System::Windows::Forms::Padding(2, 0, 2, 0);
			this->StatusLabel->Name = L"StatusLabel";
			this->StatusLabel->Size = System::Drawing::Size(41, 13);
			this->StatusLabel->TabIndex = 36;
			this->StatusLabel->Text = L"Статус";
			// 
			// StartButton
			// 
			this->StartButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->StartButton->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"StartButton.Image")));
			this->StartButton->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->StartButton->Location = System::Drawing::Point(6, 400);
			this->StartButton->Name = L"StartButton";
			this->StartButton->Size = System::Drawing::Size(83, 29);
			this->StartButton->TabIndex = 7;
			this->StartButton->Text = L"Начать";
			this->StartButton->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			this->StartButton->UseVisualStyleBackColor = true;
			this->StartButton->Click += gcnew System::EventHandler(this, &WagnerForm::StartButton_Click);
			// 
			// TgBot
			// 
			this->TgBot->BackColor = System::Drawing::SystemColors::Control;
			this->TgBot->Location = System::Drawing::Point(4, 22);
			this->TgBot->Name = L"TgBot";
			this->TgBot->Padding = System::Windows::Forms::Padding(3);
			this->TgBot->Size = System::Drawing::Size(635, 437);
			this->TgBot->TabIndex = 1;
			this->TgBot->Text = L"TgBot";
			// 
			// saveScriptFile
			// 
			this->saveScriptFile->CreatePrompt = true;
			this->saveScriptFile->DefaultExt = L"\"txt\"";
			this->saveScriptFile->Filter = L"Text files(*.txt)|*.txt";
			// 
			// openScriptFile
			// 
			this->openScriptFile->DefaultExt = L"\"txt\"";
			this->openScriptFile->FileName = L"openFileDialog1";
			this->openScriptFile->Filter = L"Text files(*.txt)|*.txt";
			// 
			// CyclogrammProgressBar
			// 
			this->CyclogrammProgressBar->BarColor = System::Drawing::Color::Lime;
			this->CyclogrammProgressBar->BorderColor = System::Drawing::Color::Black;
			this->CyclogrammProgressBar->FillStyle = ColorProgressBar::ColorProgressBar::FillStyles::Solid;
			this->CyclogrammProgressBar->Location = System::Drawing::Point(6, 62);
			this->CyclogrammProgressBar->Maximum = 100;
			this->CyclogrammProgressBar->Minimum = 0;
			this->CyclogrammProgressBar->Name = L"CyclogrammProgressBar";
			this->CyclogrammProgressBar->Size = System::Drawing::Size(623, 24);
			this->CyclogrammProgressBar->Step = 1;
			this->CyclogrammProgressBar->TabIndex = 43;
			this->CyclogrammProgressBar->Value = 0;
			// 
			// WagnerForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(834, 721);
			this->Controls->Add(this->tabControl1);
			this->Controls->Add(this->HexapodCommandListBox);
			this->Controls->Add(this->DataFrameCommandListBox);
			this->Controls->Add(this->chatTextBox);
			this->Controls->Add(this->CommandTB);
			this->Controls->Add(this->FocusCommandListBox);
			this->Controls->Add(this->ExpandButton);
			this->Controls->Add(this->ClearCyclogrammButton);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->MaximizeBox = false;
			this->Name = L"WagnerForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Wagner";
			this->Load += gcnew System::EventHandler(this, &WagnerForm::WagnerForm_Load);
			this->tabControl1->ResumeLayout(false);
			this->ScriptPage->ResumeLayout(false);
			this->ScriptPage->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->stepCountNUD))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}

#pragma endregion

#pragma region UI_Events

	private: System::Void ExpandButton_Click(System::Object^ sender, System::EventArgs^ e);

	private: System::Void StartButton_Click(System::Object^ sender, System::EventArgs^ e);

	private: System::Void PauseButton_Click(System::Object^ sender, System::EventArgs^ e);

	private: System::Void StopButton_Click(System::Object^ sender, System::EventArgs^ e);

	private: System::Void WagnerForm_Load(System::Object^ sender, System::EventArgs^ e);

	private: System::Void ClearCyclogrammButton_Click(System::Object^ sender, System::EventArgs^ e);

		   void OnDragDrop(System::Object^ sender, System::Windows::Forms::DragEventArgs^ e);

	private: System::Void FocusCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e);

	private: System::Void DataFrameCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e);

	private: System::Void HexapodCommandListBox_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e);

	private: System::Void cnctToFocus_Click(System::Object^ sender, System::EventArgs^ e);

	private: System::Void cnctToDataFrame_Click(System::Object^ sender, System::EventArgs^ e);

	private: System::Void cnctToHexapod_Click(System::Object^ sender, System::EventArgs^ e);

	private: System::Void SaveScriptBtn_Click(System::Object^ sender, System::EventArgs^ e);
		  
	private: System::Void LoadScriptBtn_Click(System::Object^ sender, System::EventArgs^ e);

#pragma endregion

#pragma region Server

		   void UpdateChatBox(String^ text);

		   void UpdateFocusConnected();
		   void UpdateFocusDisconnected();

		   void OnFocusConnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e);
		   void OnFocusDisconnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e);
		   void OnFocusDataReceived(System::Object^ sender, SuperSimpleTcp::DataReceivedEventArgs^ e);

		   void UpdateDataFrameConnected();
		   void UpdateDataFrameDisconnected();

		   void OnDataFrameConnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e);
		   void OnDataFrameDisconnected(System::Object^ sender, SuperSimpleTcp::ConnectionEventArgs^ e);
		   void OnDataFrameDataReceived(System::Object^ sender, SuperSimpleTcp::DataReceivedEventArgs^ e);

#pragma endregion

#pragma region MarshallingPackets

		   array<Byte>^ getBytes(WagnerPacket^ packet);

		   WagnerPacket^ fromBytes(array<Byte>^ arr);

#pragma endregion

#pragma region ValidateTextBox

	private: System::Void CyclogrammTextBox_Leave(System::Object^ sender, System::EventArgs^ e);

		   String^ getFunctionFromString(String^ s);

		   List<uint32_t>^ getArgsFromString(String^ s);

		   bool isFunctionValid(String^ s);

		   void ValidateText();

		   String^ getAppByFuncName(String^ funcName);

#pragma endregion

#pragma region Funcs

		   bool GetPosition(uint32_t data);

		   bool MoveTo(uint32_t data);

		   bool Park(uint32_t data);

		   bool GetErrors(uint32_t data);

		   bool ResetErrors(uint32_t data);

		   bool ParsePacket(WagnerPacket^ packet);

#pragma endregion

#pragma region Cyclogramm

		   void OnPausedHandler();

	private: System::Void DoCyclogrammWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);

	private: System::Void DoCyclogrammWorker_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e);

	private: System::Void DoCyclogrammWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);

#pragma endregion

};
}
