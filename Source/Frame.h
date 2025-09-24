#pragma once

#include <wx/frame.h>
#include <wx/aui/auibook.h>

class NotePanel;

/**
 * 
 */
class Frame : public wxFrame
{
public:
	Frame();
	virtual ~Frame();

	enum
	{
		ID_NewNote = wxID_HIGHEST,
		ID_OpenNote,
		ID_SaveNote,
		ID_Exit,
		ID_About
	};

	void UpdatePageTitleForPage(NotePanel* notePanel);

private:

	void OnNewNote(wxCommandEvent& event);
	void OnOpenNote(wxCommandEvent& event);
	void OnSaveNote(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);
	void OnPageClose(wxAuiNotebookEvent& event);

	wxAuiNotebook* noteBook;
};