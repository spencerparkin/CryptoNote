#include "NotePanel.h"
#include "App.h"
#include "Frame.h"
#include "EncryptionScheme.h"
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/filedlg.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <fstream>

NotePanel::NotePanel(wxWindow* parent) : wxPanel(parent, wxID_ANY)
{
	this->needsSave = false;

	this->textControl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	this->textControl->Bind(wxEVT_TEXT, &NotePanel::OnTextChanged, this);

	wxFont monoFont(wxFontInfo(10).FaceName("Consolas"));
	this->textControl->SetFont(monoFont);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->textControl, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);
}

/*virtual*/ NotePanel::~NotePanel()
{
}

void NotePanel::OnTextChanged(wxCommandEvent& event)
{
	this->Modified();
}

bool NotePanel::Save()
{
	if (this->filePath.empty())
	{
		wxFileDialog fileDialog(this, "Choose save location.", wxEmptyString, wxEmptyString, "Crypto Note (*.cryto_note)|*.crypto_note", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		int result = fileDialog.ShowModal();
		if (result != wxID_OK)
			return false;

		this->filePath = fileDialog.GetPath();
	}

	if (this->password.empty())
	{
		wxPasswordEntryDialog passwordDialog(this, "Choose a new password and don't forget it!", "Create Password", wxEmptyString);
		int result = passwordDialog.ShowModal();
		if (result != wxID_OK)
			return false;

		if (!this->PasswordStrongEnough(passwordDialog.GetValue()))
		{
			wxMessageBox("Password is not strong enough.", "Error!", wxOK | wxICON_ERROR, wxGetApp().GetFrame());
			return false;
		}

		this->password = passwordDialog.GetValue();
	}

	EncryptionScheme* scheme = wxGetApp().GetEncryptionScheme();
	if (!scheme)
		return false;

	std::string plainText = this->textControl->GetValue().ToStdString();
	std::vector<uint8_t> cipherText;
	if (!scheme->Encrypt(plainText, this->password.ToStdString(), cipherText))
	{
		wxMessageBox("Encryption failed!", "Error!", wxOK | wxICON_ERROR, this);
		return false;
	}

	std::ofstream fileStream;
	fileStream.open(this->filePath.ToStdString(), std::ios::out | std::ios::binary);
	if (!fileStream.is_open())
	{
		wxMessageBox(wxString::Format("Failed to open file \"%s\" for writing.", this->filePath.c_str()), "Error!", wxOK | wxICON_ERROR, this);
		return false;
	}

	fileStream.write((const char*)cipherText.data(), cipherText.size());
	fileStream.close();

	this->needsSave = false;

	return true;
}

bool NotePanel::Load()
{
	if (this->filePath.empty())
	{
		wxFileDialog fileDialog(this, "Choose open location.", wxEmptyString, wxEmptyString, "Crypto Note (*.crypto_note)|*.crypto_note", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		int result = fileDialog.ShowModal();
		if (result != wxID_OK)
			return false;

		this->filePath = fileDialog.GetPath();
	}

	if (this->password.empty())
	{
		wxPasswordEntryDialog passwordDialog(this, "Please enter the password you used to encrypt the note.", "Enter Password", wxEmptyString);
		int result = passwordDialog.ShowModal();
		if (result != wxID_OK)
			return false;

		this->password = passwordDialog.GetValue();
	}

	std::ifstream fileStream;
	fileStream.open(this->filePath.ToStdString(), std::ios::in | std::ios::binary | std::ios::ate);
	if (!fileStream.is_open())
	{
		wxMessageBox(wxString::Format("Failed to open file \"%s\" for reading.", this->filePath.c_str()), "Error!", wxOK | wxICON_ERROR, this);
		return false;
	}

	size_t fileSize = fileStream.tellg();
	std::vector<uint8_t> cipherText(fileSize);
	fileStream.seekg(0, std::ios::beg);
	fileStream.read((char*)cipherText.data(), fileSize);
	fileStream.close();

	EncryptionScheme* scheme = wxGetApp().GetEncryptionScheme();
	if (!scheme)
		return false;

	std::string plainText;
	if (!scheme->Decrypt(cipherText, this->password.ToStdString(), plainText))
	{
		wxMessageBox("Decryption failed!", "Error!", wxOK | wxICON_ERROR, this);
		return false;
	}

	this->textControl->SetValue(wxString(plainText));

	this->needsSave = false;

	return true;
}

bool NotePanel::NeedsSave()
{
	return this->needsSave;
}

wxString NotePanel::GetTabTitle()
{
	wxString title;

	if (this->filePath.empty())
		title = "Untitled";
	else
	{
		wxFileName fileName(this->filePath);
		title = fileName.GetName();
	}

	if (this->needsSave)
		title += "*";

	return title;
}

void NotePanel::Modified()
{
	if (!this->needsSave)
	{
		this->needsSave = true;
		wxGetApp().GetFrame()->UpdatePageTitleForPage(this);
	}
}

bool NotePanel::ChangePassword()
{
	wxPasswordEntryDialog oldPasswordDialog(this, "Please enter the old password.", "Enter Password", wxEmptyString);
	int result = oldPasswordDialog.ShowModal();
	if (result != wxID_OK)
		return false;

	if (oldPasswordDialog.GetValue() != this->password)
	{
		wxMessageBox("Wrong password.", "Error!", wxOK | wxICON_ERROR, wxGetApp().GetFrame());
		return false;
	}

	wxPasswordEntryDialog newPasswordDialog(this, "Please enter a new password.  Don't forget it!", "Create Password", wxEmptyString);
	result = newPasswordDialog.ShowModal();
	if (result != wxID_OK)
		return false;

	if (!this->PasswordStrongEnough(newPasswordDialog.GetValue()))
	{
		wxMessageBox("Password is not strong enough.", "Error!", wxOK | wxICON_ERROR, wxGetApp().GetFrame());
		return false;
	}

	this->password = newPasswordDialog.GetValue();
	this->Modified();
	return true;
}

bool NotePanel::PasswordStrongEnough(const wxString& passwordCandidate)
{
	if (passwordCandidate.length() < 8)
		return false;

	return true;
}