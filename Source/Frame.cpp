#include "Frame.h"
#include "NotePanel.h"
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/artprov.h>
#include <wx/msgdlg.h>
#include <wx/aboutdlg.h>

Frame::Frame() : wxFrame(nullptr, wxID_ANY, "Crypto Note", wxDefaultPosition, wxSize(1200, 1000))
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_NewNote, "New", "Create a new note."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_OpenNote, "Open", "Open an existing encpted note."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_SaveNote, "Save", "Save the current note."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Close this program."));

	fileMenu->FindItem(ID_NewNote)->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_NEW, wxART_MENU));
	fileMenu->FindItem(ID_OpenNote)->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_FILE_OPEN, wxART_MENU));
	fileMenu->FindItem(ID_SaveNote)->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_FILE_SAVE, wxART_MENU));
	fileMenu->FindItem(ID_Exit)->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_QUIT, wxART_MENU));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
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
	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_NewNote);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_OpenNote);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_SaveNote);
	this->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &Frame::OnPageClose, this);
}

/*virtual*/ Frame::~Frame()
{
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

void Frame::OnPageClose(wxAuiNotebookEvent& event)
{
	int i = event.GetSelection();
	auto notePanel = dynamic_cast<NotePanel*>(this->noteBook->GetPage(i));
	if (notePanel)
	{
		if (notePanel->NeedsSave())
		{
			int result = wxMessageBox("You have unsaved changes.  Save now before close?", "Save?", wxYES_NO | wxCANCEL | wxICON_QUESTION, this);
			if (result == wxID_YES)
			{
				if (!notePanel->Save())
					event.Veto();
			}
			else if (result == wxID_CANCEL)
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
		{
			event.Enable(this->noteBook->GetSelection() != wxNOT_FOUND);
			break;
		}
	}
}