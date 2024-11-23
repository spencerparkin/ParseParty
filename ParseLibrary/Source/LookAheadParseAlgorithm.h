#pragma once

#include "Parser.h"
#include "Grammar.h"

namespace ParseParty
{
	// This might be what is referred to in the literature as an LL(k) parser, but I'm not quite sure.
	// I might be totally wrong.  Note that here we assume that the language/grammar is unambiguous.
	class LookAheadParseAlgorithm : public Parser::Algorithm
	{
	public:
		LookAheadParseAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar);
		virtual ~LookAheadParseAlgorithm();

		virtual Parser::SyntaxNode* Parse() override;

	private:

		Parser::SyntaxNode* GenerateTree(const Grammar::Rule* grammarRule, int& parsePosition);
		int DetermineCorrectMatchSequence(const Grammar::Rule* grammarRule, int parsePosition, int lookAheadPosition, int recursionDepth = 0);
		bool TryMatchSequence(const Grammar::MatchSequence* matchSequence, int parsePosition, int lookAheadPosition, int recursionDepth);

		int lookAheadCount;
		int maxRecursionDepth;
	};
}