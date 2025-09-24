#pragma once

#include <wx/panel.h>
#include <wx/textctrl.h>

/**
 * 
 */
class NotePanel : public wxPanel
{
public:
	NotePanel(wxWindow* parent);
	virtual ~NotePanel();

	bool Save();
	bool Load();
	wxString GetTabTitle();
	bool NeedsSave();
	void Modified();
	bool ChangePassword();

private:

	void OnTextChanged(wxCommandEvent& event);
	bool PasswordStrongEnough(const wxString& passwordCandidate);

	wxString filePath;
	wxString password;
	wxTextCtrl* textControl;
	bool needsSave;
};