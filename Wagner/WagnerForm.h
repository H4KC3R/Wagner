#pragma once

namespace Wagner {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for WagnerForm
	/// </summary>
	public ref class WagnerForm : public System::Windows::Forms::Form
	{
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
	private: System::Windows::Forms::TextBox^ CommandTextBox;
	private: System::Windows::Forms::TextBox^ ServerAdrress;
	private: System::Windows::Forms::Button^ PauseButton;
	private: System::Windows::Forms::Button^ StopButton;



	private: System::Windows::Forms::Button^ StartButton;

	private: System::Windows::Forms::Button^ ExpandButton;

	private: System::Windows::Forms::ListBox^ listBox1;
	private: System::Windows::Forms::ListBox^ CommandListBox;

	private: System::Windows::Forms::TextBox^ CommandTB;
	private: System::Windows::Forms::TextBox^ textBox3;
	private: System::Windows::Forms::ProgressBar^ CyclogrammProgressBar;



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
			this->CommandTextBox = (gcnew System::Windows::Forms::TextBox());
			this->ServerAdrress = (gcnew System::Windows::Forms::TextBox());
			this->PauseButton = (gcnew System::Windows::Forms::Button());
			this->StopButton = (gcnew System::Windows::Forms::Button());
			this->StartButton = (gcnew System::Windows::Forms::Button());
			this->ExpandButton = (gcnew System::Windows::Forms::Button());
			this->listBox1 = (gcnew System::Windows::Forms::ListBox());
			this->CommandListBox = (gcnew System::Windows::Forms::ListBox());
			this->CommandTB = (gcnew System::Windows::Forms::TextBox());
			this->textBox3 = (gcnew System::Windows::Forms::TextBox());
			this->CyclogrammProgressBar = (gcnew System::Windows::Forms::ProgressBar());
			this->SuspendLayout();
			// 
			// CommandTextBox
			// 
			this->CommandTextBox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->CommandTextBox->Location = System::Drawing::Point(6, 244);
			this->CommandTextBox->Margin = System::Windows::Forms::Padding(2);
			this->CommandTextBox->Multiline = true;
			this->CommandTextBox->Name = L"CommandTextBox";
			this->CommandTextBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->CommandTextBox->Size = System::Drawing::Size(524, 308);
			this->CommandTextBox->TabIndex = 1;
			// 
			// ServerAdrress
			// 
			this->ServerAdrress->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->ServerAdrress->Location = System::Drawing::Point(182, 11);
			this->ServerAdrress->Margin = System::Windows::Forms::Padding(2);
			this->ServerAdrress->Multiline = true;
			this->ServerAdrress->Name = L"ServerAdrress";
			this->ServerAdrress->ReadOnly = true;
			this->ServerAdrress->Size = System::Drawing::Size(348, 22);
			this->ServerAdrress->TabIndex = 3;
			this->ServerAdrress->Text = L"127.0.0.1:9000";
			// 
			// PauseButton
			// 
			this->PauseButton->Location = System::Drawing::Point(6, 39);
			this->PauseButton->Name = L"PauseButton";
			this->PauseButton->Size = System::Drawing::Size(75, 23);
			this->PauseButton->TabIndex = 4;
			this->PauseButton->Text = L"Пауза";
			this->PauseButton->UseVisualStyleBackColor = true;
			// 
			// StopButton
			// 
			this->StopButton->Location = System::Drawing::Point(6, 68);
			this->StopButton->Name = L"StopButton";
			this->StopButton->Size = System::Drawing::Size(75, 23);
			this->StopButton->TabIndex = 5;
			this->StopButton->Text = L"Стоп";
			this->StopButton->UseVisualStyleBackColor = true;
			// 
			// StartButton
			// 
			this->StartButton->Location = System::Drawing::Point(6, 10);
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
			this->ExpandButton->Text = L">";
			this->ExpandButton->UseVisualStyleBackColor = true;
			this->ExpandButton->Click += gcnew System::EventHandler(this, &WagnerForm::ExpandButton_Click);
			// 
			// listBox1
			// 
			this->listBox1->FormattingEnabled = true;
			this->listBox1->Location = System::Drawing::Point(87, 11);
			this->listBox1->Name = L"listBox1";
			this->listBox1->Size = System::Drawing::Size(90, 199);
			this->listBox1->TabIndex = 9;
			// 
			// CommandListBox
			// 
			this->CommandListBox->FormattingEnabled = true;
			this->CommandListBox->Location = System::Drawing::Point(561, 41);
			this->CommandListBox->Name = L"CommandListBox";
			this->CommandListBox->Size = System::Drawing::Size(132, 511);
			this->CommandListBox->TabIndex = 10;
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
			// textBox3
			// 
			this->textBox3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 10.2F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->textBox3->Location = System::Drawing::Point(182, 39);
			this->textBox3->Margin = System::Windows::Forms::Padding(2);
			this->textBox3->Multiline = true;
			this->textBox3->Name = L"textBox3";
			this->textBox3->ReadOnly = true;
			this->textBox3->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->textBox3->Size = System::Drawing::Size(348, 171);
			this->textBox3->TabIndex = 12;
			// 
			// CyclogrammProgressBar
			// 
			this->CyclogrammProgressBar->BackColor = System::Drawing::Color::MediumBlue;
			this->CyclogrammProgressBar->ForeColor = System::Drawing::Color::ForestGreen;
			this->CyclogrammProgressBar->Location = System::Drawing::Point(6, 216);
			this->CyclogrammProgressBar->Name = L"CyclogrammProgressBar";
			this->CyclogrammProgressBar->Size = System::Drawing::Size(524, 23);
			this->CyclogrammProgressBar->TabIndex = 13;
			// 
			// WagnerForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(559, 557);
			this->Controls->Add(this->CyclogrammProgressBar);
			this->Controls->Add(this->textBox3);
			this->Controls->Add(this->CommandTB);
			this->Controls->Add(this->CommandListBox);
			this->Controls->Add(this->listBox1);
			this->Controls->Add(this->ExpandButton);
			this->Controls->Add(this->StartButton);
			this->Controls->Add(this->StopButton);
			this->Controls->Add(this->PauseButton);
			this->Controls->Add(this->ServerAdrress);
			this->Controls->Add(this->CommandTextBox);
			this->Name = L"WagnerForm";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Wagner";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void ExpandButton_Click(System::Object^ sender, System::EventArgs^ e) {
		if (ExpandButton->Text == ">") {
			this->Width = 720;
			ExpandButton->Text = "<";
		}
		else {
			this->Width = 575;
			ExpandButton->Text = ">";
		}
	}
private: System::Void StartButton_Click(System::Object^ sender, System::EventArgs^ e) {
	CyclogrammProgressBar->Increment(20);
}
};
}
