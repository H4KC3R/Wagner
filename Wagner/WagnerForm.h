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

enum Apps {
	FOCUS = 0xD0,
	DATAFRAME,
	HEXAPOD, 
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
		int StepCount = 1;
		CancellationTokenSource^ CancelReading = gcnew CancellationTokenSource();
		AutoResetEvent^ pauseEvent = gcnew AutoResetEvent(false);

	public:
		delegate bool ExecuteCommand(uint32_t data);
		delegate void Update(String^ msg);
		delegate void UpdateDisconnectionAction(int32_t app);
		bool isScriptValid = false;

		CavemanTcpClient^ FocusClient;
		CavemanTcpClient^ DataFrameClient;
		CavemanTcpClient^ HexapodClient;

		Dictionary<String^, ExecuteCommand^>^ funcs = gcnew Dictionary<String^, ExecuteCommand^>();

	private: System::Windows::Forms::Button^ cnctToDataFrame;
	private: System::Windows::Forms::Button^ cnctToFocus;
	private: System::Windows::Forms::Button^ cnctToHexapod;
	private: System::Windows::Forms::ListBox^ HexapodCommandListBox;
	private: System::Windows::Forms::TextBox^ FocusIpPortTB;
	private: System::Windows::Forms::TextBox^ DataFrameIpPortTB;
	private: System::Windows::Forms::TextBox^ HexapodIpPortTB;
	private: System::Windows::Forms::ListBox^ DataFrameCommandListBox;

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


	private: System::Windows::Forms::Button^ PauseButton;
	private: System::Windows::Forms::Button^ StopButton;
	private: System::Windows::Forms::Button^ StartButton;
	private: System::Windows::Forms::Button^ ExpandButton;
	private: System::Windows::Forms::ListBox^ FocusCommandListBox;


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
			this->StartButton = (gcnew System::Windows::Forms::Button());
			this->ExpandButton = (gcnew System::Windows::Forms::Button());
			this->FocusCommandListBox = (gcnew System::Windows::Forms::ListBox());
			this->CommandTB = (gcnew System::Windows::Forms::TextBox());
			this->chatTextBox = (gcnew System::Windows::Forms::TextBox());
			this->CyclogrammTextBox = (gcnew System::Windows::Forms::RichTextBox());
			this->ClearCyclogrammButton = (gcnew System::Windows::Forms::Button());
			this->CyclogrammProgressBar = (gcnew ProgressBarSample::TextProgressBar());
			this->DoCyclogrammWorker = (gcnew System::ComponentModel::BackgroundWorker());
			this->cnctToDataFrame = (gcnew System::Windows::Forms::Button());
			this->cnctToFocus = (gcnew System::Windows::Forms::Button());
			this->cnctToHexapod = (gcnew System::Windows::Forms::Button());
			this->DataFrameCommandListBox = (gcnew System::Windows::Forms::ListBox());
			this->HexapodCommandListBox = (gcnew System::Windows::Forms::ListBox());
			this->FocusIpPortTB = (gcnew System::Windows::Forms::TextBox());
			this->DataFrameIpPortTB = (gcnew System::Windows::Forms::TextBox());
			this->HexapodIpPortTB = (gcnew System::Windows::Forms::TextBox());
			this->SuspendLayout();
			// 
			// PauseButton
			// 
			this->PauseButton->Enabled = false;
			this->PauseButton->Location = System::Drawing::Point(6, 205);
			this->PauseButton->Name = L"PauseButton";
			this->PauseButton->Size = System::Drawing::Size(75, 23);
			this->PauseButton->TabIndex = 4;
			this->PauseButton->Text = L"Пауза";
			this->PauseButton->UseVisualStyleBackColor = true;
			this->PauseButton->Click += gcnew System::EventHandler(this, &WagnerForm::PauseButton_Click);
			// 
			// StopButton
			// 
			this->StopButton->Enabled = false;
			this->StopButton->Location = System::Drawing::Point(87, 176);
			this->StopButton->Name = L"StopButton";
			this->StopButton->Size = System::Drawing::Size(75, 23);
			this->StopButton->TabIndex = 5;
			this->StopButton->Text = L"Стоп";
			this->StopButton->UseVisualStyleBackColor = true;
			this->StopButton->Click += gcnew System::EventHandler(this, &WagnerForm::StopButton_Click);
			// 
			// StartButton
			// 
			this->StartButton->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->StartButton->Location = System::Drawing::Point(6, 176);
			this->StartButton->Name = L"StartButton";
			this->StartButton->Size = System::Drawing::Size(75, 23);
			this->StartButton->TabIndex = 7;
			this->StartButton->Text = L"Начать";
			this->StartButton->UseVisualStyleBackColor = true;
			this->StartButton->Click += gcnew System::EventHandler(this, &WagnerForm::StartButton_Click);
			// 
			// ExpandButton
			// 
			this->ExpandButton->Location = System::Drawing::Point(535, 183);
			this->ExpandButton->Name = L"ExpandButton";
			this->ExpandButton->Size = System::Drawing::Size(20, 168);
			this->ExpandButton->TabIndex = 8;
			this->ExpandButton->Text = L"<";
			this->ExpandButton->UseVisualStyleBackColor = true;
			this->ExpandButton->Click += gcnew System::EventHandler(this, &WagnerForm::ExpandButton_Click);
			// 
			// FocusCommandListBox
			// 
			this->FocusCommandListBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->FocusCommandListBox->FormattingEnabled = true;
			this->FocusCommandListBox->ItemHeight = 16;
			this->FocusCommandListBox->Items->AddRange(gcnew cli::array< System::Object^  >(5) {
				L"getPosition", L"moveTo", L"park", L"getErrors",
					L"resetErrors"
			});
			this->FocusCommandListBox->Location = System::Drawing::Point(561, 41);
			this->FocusCommandListBox->Name = L"FocusCommandListBox";
			this->FocusCommandListBox->Size = System::Drawing::Size(132, 164);
			this->FocusCommandListBox->TabIndex = 10;
			this->FocusCommandListBox->SelectedIndexChanged += gcnew System::EventHandler(this, &WagnerForm::FocusCommandListBox_SelectedIndexChanged);
			// 
			// CommandTB
			// 
			this->CommandTB->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->CommandTB->Location = System::Drawing::Point(561, 11);
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
			this->chatTextBox->Location = System::Drawing::Point(167, 12);
			this->chatTextBox->Margin = System::Windows::Forms::Padding(2);
			this->chatTextBox->Multiline = true;
			this->chatTextBox->Name = L"chatTextBox";
			this->chatTextBox->ReadOnly = true;
			this->chatTextBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->chatTextBox->Size = System::Drawing::Size(363, 216);
			this->chatTextBox->TabIndex = 12;
			// 
			// CyclogrammTextBox
			// 
			this->CyclogrammTextBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->CyclogrammTextBox->Location = System::Drawing::Point(6, 262);
			this->CyclogrammTextBox->Name = L"CyclogrammTextBox";
			this->CyclogrammTextBox->Size = System::Drawing::Size(525, 292);
			this->CyclogrammTextBox->TabIndex = 14;
			this->CyclogrammTextBox->Text = L" ";
			this->CyclogrammTextBox->Leave += gcnew System::EventHandler(this, &WagnerForm::CyclogrammTextBox_Leave);
			// 
			// ClearCyclogrammButton
			// 
			this->ClearCyclogrammButton->Location = System::Drawing::Point(87, 205);
			this->ClearCyclogrammButton->Name = L"ClearCyclogrammButton";
			this->ClearCyclogrammButton->Size = System::Drawing::Size(75, 23);
			this->ClearCyclogrammButton->TabIndex = 15;
			this->ClearCyclogrammButton->Text = L"Очистить";
			this->ClearCyclogrammButton->UseVisualStyleBackColor = true;
			this->ClearCyclogrammButton->Click += gcnew System::EventHandler(this, &WagnerForm::ClearCyclogrammButton_Click);
			// 
			// CyclogrammProgressBar
			// 
			this->CyclogrammProgressBar->CustomText = L"";
			this->CyclogrammProgressBar->Location = System::Drawing::Point(6, 233);
			this->CyclogrammProgressBar->Margin = System::Windows::Forms::Padding(2);
			this->CyclogrammProgressBar->Name = L"CyclogrammProgressBar";
			this->CyclogrammProgressBar->ProgressColor = System::Drawing::Color::LightGreen;
			this->CyclogrammProgressBar->Size = System::Drawing::Size(524, 24);
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
			this->DoCyclogrammWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &WagnerForm::DoCyclogrammWorker_RunWorkerCompleted);
			// 
			// cnctToDataFrame
			// 
			this->cnctToDataFrame->ForeColor = System::Drawing::Color::DarkGreen;
			this->cnctToDataFrame->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->cnctToDataFrame->Location = System::Drawing::Point(6, 92);
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
			this->cnctToFocus->Location = System::Drawing::Point(6, 37);
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
			this->cnctToHexapod->Location = System::Drawing::Point(6, 147);
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
			this->DataFrameCommandListBox->Location = System::Drawing::Point(561, 215);
			this->DataFrameCommandListBox->Name = L"DataFrameCommandListBox";
			this->DataFrameCommandListBox->Size = System::Drawing::Size(132, 164);
			this->DataFrameCommandListBox->TabIndex = 20;
			this->DataFrameCommandListBox->SelectedIndexChanged += gcnew System::EventHandler(this, &WagnerForm::DataFrameCommandListBox_SelectedIndexChanged);
			// 
			// HexapodCommandListBox
			// 
			this->HexapodCommandListBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
			this->HexapodCommandListBox->FormattingEnabled = true;
			this->HexapodCommandListBox->ItemHeight = 16;
			this->HexapodCommandListBox->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"moveLinear", L"moveAngular", L"moveToZero" });
			this->HexapodCommandListBox->Location = System::Drawing::Point(560, 390);
			this->HexapodCommandListBox->Name = L"HexapodCommandListBox";
			this->HexapodCommandListBox->Size = System::Drawing::Size(132, 164);
			this->HexapodCommandListBox->TabIndex = 21;
			this->HexapodCommandListBox->SelectedIndexChanged += gcnew System::EventHandler(this, &WagnerForm::HexapodCommandListBox_SelectedIndexChanged);
			// 
			// FocusIpPortTB
			// 
			this->FocusIpPortTB->Location = System::Drawing::Point(6, 11);
			this->FocusIpPortTB->Name = L"FocusIpPortTB";
			this->FocusIpPortTB->ReadOnly = true;
			this->FocusIpPortTB->Size = System::Drawing::Size(156, 20);
			this->FocusIpPortTB->TabIndex = 22;
			this->FocusIpPortTB->Text = L"127.0.0.1:9000";
			// 
			// DataFrameIpPortTB
			// 
			this->DataFrameIpPortTB->Location = System::Drawing::Point(6, 66);
			this->DataFrameIpPortTB->Name = L"DataFrameIpPortTB";
			this->DataFrameIpPortTB->ReadOnly = true;
			this->DataFrameIpPortTB->Size = System::Drawing::Size(156, 20);
			this->DataFrameIpPortTB->TabIndex = 23;
			// 
			// HexapodIpPortTB
			// 
			this->HexapodIpPortTB->Location = System::Drawing::Point(6, 121);
			this->HexapodIpPortTB->Name = L"HexapodIpPortTB";
			this->HexapodIpPortTB->ReadOnly = true;
			this->HexapodIpPortTB->Size = System::Drawing::Size(156, 20);
			this->HexapodIpPortTB->TabIndex = 24;
			this->HexapodIpPortTB->Text = L"127.0.0.1:5000";
			// 
			// WagnerForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(704, 557);
			this->Controls->Add(this->HexapodIpPortTB);
			this->Controls->Add(this->DataFrameIpPortTB);
			this->Controls->Add(this->FocusIpPortTB);
			this->Controls->Add(this->HexapodCommandListBox);
			this->Controls->Add(this->DataFrameCommandListBox);
			this->Controls->Add(this->cnctToHexapod);
			this->Controls->Add(this->cnctToFocus);
			this->Controls->Add(this->cnctToDataFrame);
			this->Controls->Add(this->CyclogrammProgressBar);
			this->Controls->Add(this->ClearCyclogrammButton);
			this->Controls->Add(this->CyclogrammTextBox);
			this->Controls->Add(this->chatTextBox);
			this->Controls->Add(this->CommandTB);
			this->Controls->Add(this->FocusCommandListBox);
			this->Controls->Add(this->ExpandButton);
			this->Controls->Add(this->StartButton);
			this->Controls->Add(this->StopButton);
			this->Controls->Add(this->PauseButton);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->MaximizeBox = false;
			this->Name = L"WagnerForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Wagner";
			this->Load += gcnew System::EventHandler(this, &WagnerForm::WagnerForm_Load);
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

#pragma endregion

#pragma region Server
		   
		   void UpdateChatBox(String^ text);
		   void UpdateClientDisconnected(int32_t app);

		   void OnFocusDisconnected(System::Object^ sender, System::EventArgs^ e);
		   void OnHexapodDisconnected(System::Object^ sender, System::EventArgs^ e);

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

		   bool GetPosition(uint32_t data);

		   bool MoveTo(uint32_t data);

		   bool Park(uint32_t data);

		   bool GetErrors(uint32_t data);

		   bool ResetErrors(uint32_t data);

#pragma endregion

#pragma region Cyclogramm

	private: System::Void DoCyclogrammWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);

	private: System::Void DoCyclogrammWorker_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e);
	
	private: System::Void DoCyclogrammWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);

#pragma endregion

};
}
