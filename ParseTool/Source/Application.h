#pragma once

#include <wx/app.h>
#include "Grammar.h"
#include "Parser.h"

class Frame;

class Application : public wxApp
{
public:
	Application();
	virtual ~Application();

	virtual bool OnInit(void) override;
	virtual int OnExit(void) override;

	Frame* frame;

	ParseParty::Grammar grammar;
	ParseParty::Parser parser;

	ParseParty::Parser::SyntaxNode* rootNode;
};

wxDECLARE_APP(Application);