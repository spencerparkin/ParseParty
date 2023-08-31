#pragma once

#include <wx/frame.h>
#include <wx/treectrl.h>

class Frame : public wxFrame
{
public:
	Frame(wxWindow* parent, const wxPoint& pos, const wxSize& size);
	virtual ~Frame();

	void OnExit(wxCommandEvent& event);
	void OnGrammarFile(wxCommandEvent& event);
	void OnLexiconFile(wxCommandEvent& event);
	void OnSyntaxTreeFile(wxCommandEvent& event);
	void OnParseFile(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnUpdateUI(wxUpdateUIEvent& event);

	enum
	{
		ID_Exit = wxID_HIGHEST,
		ID_ReadGrammarFile,
		ID_WriteGrammarFile,
		ID_ReadLexiconFile,
		ID_WriteLexiconFile,
		ID_ReadSyntaxTreeFile,
		ID_WriteSyntaxTreeFile,
		ID_ParseFile,
		ID_About
	};

	void RebuildTreeControl();

	wxTreeCtrl* treeControl;
};