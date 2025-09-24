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

private:

	void OnTextChanged(wxCommandEvent& event);

	wxString filePath;
	wxString password;
	wxTextCtrl* textControl;
	bool needsSave;
};