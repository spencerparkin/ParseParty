#pragma once

#include "Parser.h"

namespace ParseParty
{
	// This algorithm is designed to be an improvement over the SlowParseAlgorithm.
	// Specifically, it should be able to handle adjacent non-terminals.  It makes the
	// same assumption that paran/brackets of all sorts are balanced, and that terminals
	// match at the same nesting level.
	class GeneralParseAlgorithm : public Parser::Algorithm
	{
	public:
		GeneralParseAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar);
		virtual ~GeneralParseAlgorithm();

		virtual Parser::SyntaxNode* Parse() override;
	};
}