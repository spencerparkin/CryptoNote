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

private:

	wxString filePath;
	wxTextCtrl* textControl;
	bool needsSave;
};