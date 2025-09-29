#include "Frame.h"
#include "NotePanel.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/aboutdlg.h>
#include <wx/accel.h>
#include <wx/textdlg.h>

Frame::Frame() : wxFrame(nullptr, wxID_ANY, "Crypto Note", wxDefaultPosition, wxSize(1200, 1000))
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_NewNote, "New\tCtrl+N", "Create a new note."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_OpenNote, "Open\tCtrl+O", "Open an existing encpted note."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SaveNote, "Save\tCtrl+S", "Save the current note."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Close this program."));

	wxMenu* editMenu = new wxMenu();
	editMenu->Append(new wxMenuItem(editMenu, ID_Find, "Find\tCtrl+F", "Find text in the current note."));
	editMenu->Append(new wxMenuItem(editMenu, ID_FindAndReplace, "Find And Replace", "Replace all occurrances of certain text with other text."));
	editMenu->AppendSeparator();
	editMenu->Append(new wxMenuItem(editMenu, ID_RepeatLastFind, "Repeat Find\tF3", "Repleat the last find operation from the current cursor location."));

	wxMenu* optionsMenu = new wxMenu();
	optionsMenu->Append(new wxMenuItem(optionsMenu, ID_ChangePassword, "Change Password", "Change the password used on the current note."));

	fileMenu->FindItem(ID_NewNote)->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_NEW, wxART_MENU));
	fileMenu->FindItem(ID_OpenNote)->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_FILE_OPEN, wxART_MENU));
	fileMenu->FindItem(ID_SaveNote)->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_FILE_SAVE, wxART_MENU));
	fileMenu->FindItem(ID_Exit)->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_QUIT, wxART_MENU));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(editMenu, "Edit");
	menuBar->Append(optionsMenu, "Options");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	wxStatusBar* statusBar = new wxStatusBar(this);
	this->SetStatusBar(statusBar);

	this->noteBook = new wxAuiNotebook(this, wxID_ANY);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(this->noteBook, 1, wxALL | wxGROW, 0);
	this->SetSizer(sizer);

	this->Bind(wxEVT_MENU, &Frame::OnNewNote, this, ID_NewNote);
	this->Bind(wxEVT_MENU, &Frame::OnOpenNote, this, ID_OpenNote);
	this->Bind(wxEVT_MENU, &Frame::OnSaveNote, this, ID_SaveNote);
	this->Bind(wxEVT_MENU, &Frame::OnChangePassword, this, ID_ChangePassword);
	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnFind, this, ID_Find);
	this->Bind(wxEVT_MENU, &Frame::OnFind, this, ID_FindAndReplace);
	this->Bind(wxEVT_MENU, &Frame::OnFind, this, ID_RepeatLastFind);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_NewNote);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_OpenNote);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_SaveNote);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ChangePassword);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_Find);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_FindAndReplace);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_RepeatLastFind);
	this->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &Frame::OnPageClose, this);
	this->Bind(wxEVT_CLOSE_WINDOW, &Frame::OnCloseWindow, this);

	std::vector<wxAcceleratorEntry> accelEntryArray;
	accelEntryArray.push_back(wxAcceleratorEntry(wxACCEL_CTRL, (int)'S', ID_SaveNote));
	accelEntryArray.push_back(wxAcceleratorEntry(wxACCEL_CTRL, (int)'N', ID_NewNote));
	accelEntryArray.push_back(wxAcceleratorEntry(wxACCEL_CTRL, (int)'O', ID_OpenNote));
	accelEntryArray.push_back(wxAcceleratorEntry(wxACCEL_CTRL, (int)'F', ID_Find));
	accelEntryArray.push_back(wxAcceleratorEntry(wxACCEL_NORMAL, WXK_F3, ID_RepeatLastFind));

	wxAcceleratorTable accelTable((int)accelEntryArray.size(), accelEntryArray.data());
	this->SetAcceleratorTable(accelTable);
}

/*virtual*/ Frame::~Frame()
{
}

void Frame::OnFind(wxCommandEvent& event)
{
	int i = this->noteBook->GetSelection();
	auto notePanel = dynamic_cast<NotePanel*>(this->noteBook->GetPage(i));
	if (!notePanel)
		return;

	if (event.GetId() == ID_Find || event.GetId() == ID_FindAndReplace)
	{
		this->searchText = notePanel->GetSelectedText();
		wxTextEntryDialog searchTextDialog(this, "Enter search text.", "Find", searchText);
		if (searchTextDialog.ShowModal() != wxID_OK)
			return;

		this->searchText = searchTextDialog.GetValue();
		if (this->searchText.empty())
			return;
	}

	if (event.GetId() == ID_FindAndReplace)
	{
		wxTextEntryDialog replaceTextDialog(this, "Enter replacement text.", "Replace", this->searchText);
		if (replaceTextDialog.ShowModal() != wxID_OK)
			return;

		wxString replaceText = replaceTextDialog.GetValue();
		uint32_t numReplacements = notePanel->SearchAndReplace(this->searchText, replaceText);
		wxMessageBox(wxString::Format("Made %d replacements.", numReplacements), "Find & Replace", wxOK | wxICON_INFORMATION, this);
	}
	else
	{
		if (!notePanel->HighlightNextMatch(this->searchText))
		{
			wxMessageBox(wxString::Format("No more instances of \"%s\" could be found in the text.", this->searchText.c_str()), "Find", wxOK | wxICON_INFORMATION, this);
		}
	}
}

void Frame::OnChangePassword(wxCommandEvent& event)
{
	int i = this->noteBook->GetSelection();
	auto notePanel = dynamic_cast<NotePanel*>(this->noteBook->GetPage(i));
	if (!notePanel)
		return;

	notePanel->ChangePassword();
}

void Frame::OnNewNote(wxCommandEvent& event)
{
	auto notePanel = new NotePanel(this->noteBook);
	this->noteBook->AddPage(notePanel, notePanel->GetTabTitle());
}

void Frame::OnOpenNote(wxCommandEvent& event)
{
	this->Freeze();

	std::unique_ptr<NotePanel> notePanel(new NotePanel(this->noteBook));
	if (notePanel->Load())
	{
		wxString title = notePanel->GetTabTitle();
		this->noteBook->AddPage(notePanel.release(), title, true);
	}

	this->Thaw();
}

void Frame::OnSaveNote(wxCommandEvent& event)
{
	int i = this->noteBook->GetSelection();
	auto notePanel = dynamic_cast<NotePanel*>(this->noteBook->GetPage(i));
	if (!notePanel)
		return;

	if (notePanel->Save())
		this->noteBook->SetPageText(i, notePanel->GetTabTitle());
}

void Frame::UpdatePageTitleForPage(NotePanel* notePanel)
{
	int i = this->noteBook->FindPage(notePanel);
	if (i != wxNOT_FOUND)
		this->noteBook->SetPageText(i, notePanel->GetTabTitle());
}

void Frame::OnCloseWindow(wxCloseEvent& event)
{
	bool unsavedChanges = false;
	for (int i = 0; i < (int)this->noteBook->GetPageCount(); i++)
	{
		auto notePanel = dynamic_cast<NotePanel*>(this->noteBook->GetPage(i));
		if (notePanel && notePanel->NeedsSave())
		{
			unsavedChanges = true;
			break;
		}
	}

	if (unsavedChanges)
	{
		int result = wxMessageBox("One or more open notes have unsaved changes.  Close anyway?", "Close?", wxYES_NO | wxICON_QUESTION, this);
		if (result == wxNO)
		{
			event.Veto();
			return;
		}
	}

	event.Skip();
}

void Frame::OnPageClose(wxAuiNotebookEvent& event)
{
	int i = event.GetSelection();
	auto notePanel = dynamic_cast<NotePanel*>(this->noteBook->GetPage(i));
	if (notePanel)
	{
		if (notePanel->NeedsSave())
		{
			int result = wxMessageBox("You have unsaved changes.  Save now before close?", "Save?", wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
			if (result == wxYES)
			{
				if (!notePanel->Save())
					event.Veto();
			}
			else if (result == wxCANCEL)
			{
				event.Veto();
			}
		}
	}
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Crypto Note");
	aboutDialogInfo.SetDescription("This is a basic notepad application with AES encryption support.");
	aboutDialogInfo.SetCopyright("(c) 2025, Spencer T. Parkin");

	wxAboutBox(aboutDialogInfo);
}

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_NewNote:
		case ID_OpenNote:
		{
			event.Enable(true);
			break;
		}
		case ID_SaveNote:
		case ID_ChangePassword:
		case ID_Find:
		case ID_FindAndReplace:
		{
			event.Enable(this->noteBook->GetSelection() != wxNOT_FOUND);
			break;
		}
		case ID_RepeatLastFind:
		{
			event.Enable(this->noteBook->GetSelection() != wxNOT_FOUND && this->searchText.size() > 0);
			break;
		}
	}
}