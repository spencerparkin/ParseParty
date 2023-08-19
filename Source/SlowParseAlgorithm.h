#pragma once

#include "Parser.h"

namespace ParseParty
{
	// This algorithm isn't going to scale well with the size the input, but it's the best I can come up with.
	// It makes only one assumption as additional information alongside the given token array and grammar, and
	// it is that all parenthesis, curly braces and square brackets are meant to be balanced.
	class SlowParseAlgorithm : public Parser::Algorithm
	{
	public:
		SlowParseAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar);
		virtual ~SlowParseAlgorithm();

		virtual Parser::SyntaxNode* Parse() override;
	};
}