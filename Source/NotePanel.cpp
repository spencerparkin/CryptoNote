#include "NotePanel.h"
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/filedlg.h>

NotePanel::NotePanel(wxWindow* parent) : wxPanel(parent, wxID_ANY)
{
	this->needsSave = false;

	this->textControl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);

	wxFont monoFont(wxFontInfo(10).FaceName("Consolas"));
	this->textControl->SetFont(monoFont);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(this->textControl, 1, wxALL | wxGROW, 0);
	this->SetSizer(boxSizer);
}

/*virtual*/ NotePanel::~NotePanel()
{
}

bool NotePanel::Save()
{
	if (this->filePath.empty())
	{
		wxFileDialog fileDialog(this, "Choose save location.", wxEmptyString, wxEmptyString, "Crypto Note *.cryto_note|(*.crypto_note)", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		int result = fileDialog.ShowModal();
		if (result != wxID_OK)
			return false;

		this->filePath = fileDialog.GetPath();
	}

	// STPTODO: Write this.

	return true;
}

bool NotePanel::Load()
{
	if (this->filePath.empty())
	{
		wxFileDialog fileDialog(this, "Choose open location.", wxEmptyString, wxEmptyString, "Crypto Note *.crypto_note|(*.crypto_note)", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		int result = fileDialog.ShowModal();
		if (result != wxID_OK)
			return false;

		this->filePath = fileDialog.GetPath();
	}

	// STPTODO: Write this.

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
		title = fileName.GetFullName();
	}

	if (this->needsSave)
		title += "*";

	return title;
}