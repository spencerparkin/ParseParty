#include "Frame.h"
#include <wx/menu.h>
#include <wx/aboutdlg.h>

Frame::Frame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Parse Tool", pos, size)
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ReadGrammarFile, "Read Grammar File", "Read in a grammar definition to be used during parsing."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_WriteGrammarFile, "Write Grammar File", "Write out the current grammar definition to disk."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ParseFile, "Parse File", "Parse the given file against the current grammar definition."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_Exit, "Exit", "Get out of here!"));

	wxMenu* helpMenu = new wxMenu();
	helpMenu->Append(new wxMenuItem(helpMenu, ID_About, "About", "Show the about-box."));

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(fileMenu, "File");
	menuBar->Append(helpMenu, "Help");
	this->SetMenuBar(menuBar);

	this->SetStatusBar(new wxStatusBar(this));

	this->Bind(wxEVT_MENU, &Frame::OnExit, this, ID_Exit);
	this->Bind(wxEVT_MENU, &Frame::OnAbout, this, ID_About);
	this->Bind(wxEVT_MENU, &Frame::OnGrammarFile, this, ID_ReadGrammarFile);
	this->Bind(wxEVT_MENU, &Frame::OnGrammarFile, this, ID_WriteGrammarFile);
	this->Bind(wxEVT_MENU, &Frame::OnParseFile, this, ID_ParseFile);
}

/*virtual*/ Frame::~Frame()
{
}

void Frame::OnExit(wxCommandEvent& event)
{
	this->Close(true);
}

void Frame::OnGrammarFile(wxCommandEvent& event)
{
}

void Frame::OnParseFile(wxCommandEvent& event)
{
}

void Frame::OnAbout(wxCommandEvent& event)
{
	wxAboutDialogInfo aboutDialogInfo;

	aboutDialogInfo.SetName("Parse Tool");
	aboutDialogInfo.SetVersion("1.0");
	aboutDialogInfo.SetDescription("This tool helps you develop grammars that can be used to parse code into abstract syntax trees.");
	aboutDialogInfo.SetCopyright("Copyright (C) 2023 -- Spencer T. Parkin <SpencerTParkin@gmail.com>");

	wxAboutBox(aboutDialogInfo);
}