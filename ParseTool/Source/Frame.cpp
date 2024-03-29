#include "Frame.h"
#include "Application.h"
#include "FormatString.h"
#include <wx/menu.h>
#include <wx/aboutdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/time.h>

Frame::Frame(wxWindow* parent, const wxPoint& pos, const wxSize& size) : wxFrame(parent, wxID_ANY, "Parse Tool", pos, size)
{
	wxMenu* fileMenu = new wxMenu();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ReadGrammarFile, "Read Grammar File", "Read in a grammar definition to be used during parsing."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_WriteGrammarFile, "Write Grammar File", "Write out the current grammar definition to disk."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ReadLexiconFile, "Read Lexicon File", "Read in a lexicon configuration file used during lexical analysis."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_WriteLexiconFile, "Write Lexicon File", "Write out the current lexicon configuration to disk."));
	fileMenu->AppendSeparator();
	fileMenu->Append(new wxMenuItem(fileMenu, ID_ReadSyntaxTreeFile, "Read Syntax Tree File", "Read in a file containing an entire AST."));
	fileMenu->Append(new wxMenuItem(fileMenu, ID_WriteSyntaxTreeFile, "Write Syntax Tree File", "Write the currently generated AST to file."));
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
	this->Bind(wxEVT_MENU, &Frame::OnLexiconFile, this, ID_ReadLexiconFile);
	this->Bind(wxEVT_MENU, &Frame::OnLexiconFile, this, ID_WriteLexiconFile);
	this->Bind(wxEVT_MENU, &Frame::OnSyntaxTreeFile, this, ID_ReadSyntaxTreeFile);
	this->Bind(wxEVT_MENU, &Frame::OnSyntaxTreeFile, this, ID_WriteSyntaxTreeFile);
	this->Bind(wxEVT_MENU, &Frame::OnParseFile, this, ID_ParseFile);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_ParseFile);
	this->Bind(wxEVT_UPDATE_UI, &Frame::OnUpdateUI, this, ID_WriteSyntaxTreeFile);

	this->treeControl = new wxTreeCtrl(this);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(this->treeControl, 1, wxGROW | wxALL, 0);
	this->SetSizer(sizer);
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
	switch (event.GetId())
	{
		case ID_ReadGrammarFile:
		{
			wxFileDialog fileDialog(this, "Open Grammar File", wxEmptyString, wxEmptyString, "JSON file (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (wxID_OK == fileDialog.ShowModal())
			{
				wxBusyCursor busyCursor;
				wxString grammarFile = fileDialog.GetPath();
				std::string error;
				if (!wxGetApp().grammar.ReadFile((const char*)grammarFile.c_str(), error))
					wxMessageBox("Failed to open grammar file: " + grammarFile + "\n\n" + wxString(error.c_str()), "Error!", wxICON_ERROR | wxOK, this);
				else
					wxMessageBox("Grammar file read!", "Success!", wxICON_INFORMATION | wxOK, this);
			}
			break;
		}
		case ID_WriteGrammarFile:
		{
			wxFileDialog fileDialog(this, "Save Grammar File", wxEmptyString, wxEmptyString, "JSON file (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (wxID_OK == fileDialog.ShowModal())
			{
				wxBusyCursor busyCursor;
				wxString grammarFile = fileDialog.GetPath();
				if (!wxGetApp().grammar.WriteFile((const char*)grammarFile.c_str()))
					wxMessageBox("Failed to save grammar file: " + grammarFile, "Error!", wxICON_ERROR | wxOK, this);
				else
					wxMessageBox("Grammar file written!", "Success!", wxICON_INFORMATION | wxOK, this);
			}

			break;
		}
	}
}

void Frame::OnLexiconFile(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case ID_ReadLexiconFile:
		{
			wxFileDialog fileDialog(this, "Open Lexicon File", wxEmptyString, wxEmptyString, "JSON file (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (wxID_OK == fileDialog.ShowModal())
			{
				wxBusyCursor busyCursor;
				wxString lexiconFile = fileDialog.GetPath();
				std::string error;
				if (!wxGetApp().parser.lexer.ReadFile((const char*)lexiconFile.c_str(), error))
					wxMessageBox("Failed to open lexicon file: " + lexiconFile + "\n\n" + wxString(error.c_str()), "Error!", wxICON_ERROR | wxOK, this);
				else
					wxMessageBox("Lexicon file read!", "Success!", wxICON_INFORMATION | wxOK, this);
			}
			break;
		}
		case ID_WriteLexiconFile:
		{
			wxFileDialog fileDialog(this, "Save Lexicon File", wxEmptyString, wxEmptyString, "JSON file (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (wxID_OK == fileDialog.ShowModal())
			{
				wxBusyCursor busyCursor;
				wxString lexiconFile = fileDialog.GetPath();
				if (!wxGetApp().parser.lexer.WriteFile((const char*)lexiconFile.c_str()))
					wxMessageBox("Failed to save lexicon file: " + lexiconFile, "Error!", wxICON_ERROR | wxOK, this);
				else
					wxMessageBox("Lexicon file written!", "Success!", wxICON_INFORMATION | wxOK, this);
			}
			break;
		}
	}
}

void Frame::OnSyntaxTreeFile(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case ID_ReadSyntaxTreeFile:
		{
			wxFileDialog fileDialog(this, "Open AST File", wxEmptyString, wxEmptyString, "JSON file (*.json)|*.json", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
			if (wxID_OK == fileDialog.ShowModal())
			{
				wxBusyCursor busyCursor;
				wxString syntaxTreeFile = fileDialog.GetPath();
				std::string error;
				if (!ParseParty::Parser::SyntaxNode::ReadFromFile((const char*)syntaxTreeFile.c_str(), wxGetApp().rootNode, error))
					wxMessageBox("Failed to read AST file: " + syntaxTreeFile + "\n\n" + error, "Error!", wxICON_ERROR | wxOK, this);
				else
				{
					wxMessageBox("AST file read!", "Success!", wxICON_INFORMATION | wxOK, this);
					this->RebuildTreeControl();
				}
			}
			break;
		}
		case ID_WriteSyntaxTreeFile:
		{
			wxFileDialog fileDialog(this, "Save AST File", wxEmptyString, wxEmptyString, "JSON file (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
			if (wxID_OK == fileDialog.ShowModal())
			{
				wxBusyCursor busyCursor;
				wxString syntaxTreeFile = fileDialog.GetPath();
				if (!ParseParty::Parser::SyntaxNode::WriteToFile((const char*)syntaxTreeFile.c_str(), wxGetApp().rootNode))
					wxMessageBox("Failed to write AST file: " + syntaxTreeFile, "Error!", wxICON_ERROR | wxOK, this);
				else
					wxMessageBox("AST file written!", "Success!", wxICON_INFORMATION | wxOK, this);
			}
			break;
		}
	}
}

void Frame::OnParseFile(wxCommandEvent& event)
{
	wxFileDialog fileDialog(this, "Open Code File", wxEmptyString, wxEmptyString, "Any File (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (wxID_OK == fileDialog.ShowModal())
	{
		wxBusyCursor busyCursor;

		wxString codeFile = fileDialog.GetPath();
		delete wxGetApp().rootNode;

		std::string parseError;

		wxLongLong parseTimeBegin = wxGetLocalTimeMillis();
		wxGetApp().rootNode = wxGetApp().parser.ParseFile((const char*)codeFile.c_str(), wxGetApp().grammar, &parseError);
		wxLongLong parseTimeEnd = wxGetLocalTimeMillis();
		wxLongLong parseTimeElapsed = parseTimeEnd - parseTimeBegin;

		if (!wxGetApp().rootNode)
		{
			wxString errorMsg(parseError.c_str());
			wxMessageBox("Failed to parse file: " + codeFile + "\n\nError: " + errorMsg, "Error!", wxICON_ERROR | wxOK, this);
		}
		else
		{
			wxMessageBox(wxString::Format("Parse took %llu milliseconds", parseTimeElapsed), "Parse Complete!", wxICON_INFORMATION | wxOK, this);

			this->RebuildTreeControl();
		}
	}
}

// TODO: We really need to use a better tree control; one with columns and column headers.
void Frame::RebuildTreeControl()
{
	this->treeControl->DeleteAllItems();

	if (wxGetApp().rootNode)
	{
		wxTreeItemId rootItemId = this->treeControl->AddRoot(wxGetApp().rootNode->text->c_str());

		struct Node
		{
			wxTreeItemId parentItemId;
			const ParseParty::Parser::SyntaxNode* parentNode;
		};

		std::list<Node> nodeQueue;
		nodeQueue.push_back(Node{ rootItemId, wxGetApp().rootNode });
		while (nodeQueue.size() > 0)
		{
			std::list<Node>::iterator iter = nodeQueue.begin();
			Node node = *iter;
			nodeQueue.erase(iter);

			for (const ParseParty::Parser::SyntaxNode* childNode : *node.parentNode->childList)
			{
				std::string text = ParseParty::FormatString("%s -- (line %d, column %d)", childNode->text->c_str(), childNode->fileLocation.line, childNode->fileLocation.column);
				wxTreeItemId childItemId = this->treeControl->AppendItem(node.parentItemId, text.c_str());
				nodeQueue.push_back(Node{ childItemId, childNode });
			}
		}

		this->treeControl->ExpandAllChildren(rootItemId);
	}
}

void Frame::OnUpdateUI(wxUpdateUIEvent& event)
{
	switch (event.GetId())
	{
		case ID_ParseFile:
		{
			event.Enable(wxGetApp().grammar.ruleMap->size() > 0);
			break;
		}
		case ID_WriteSyntaxTreeFile:
		{
			event.Enable(wxGetApp().rootNode != nullptr);
			break;
		}
	}
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