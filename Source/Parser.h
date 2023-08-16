#pragma once

#include "Common.h"
#include "Lexer.h"
#include "Grammar.h"

namespace ParseParty
{
	class PARSE_PARTY_API Parser
	{
	public:
		Parser();
		virtual ~Parser();

		class SyntaxNode;

		// The goal here is to provide an algorithm that, for some grammars, parses in linear time.
		// For other grammars, performance should be reasonably improved through the use of something
		// close to the concept of memoization.  Memory copies should be minimally performed, if not
		// eliminated altogether.  Any successfully parsed sub-region of the token array should get
		// cached for potential re-use if it is to be discarded during the parsing process.
		SyntaxNode* Parse(const std::vector<Lexer::Token*>& tokenArray, const Grammar& grammar);

		class PARSE_PARTY_API SyntaxNode
		{
		public:
			SyntaxNode();
			virtual ~SyntaxNode();

			void WipeChildren();

			SyntaxNode* parentNode;
			std::vector<SyntaxNode*>* childArray;
			std::string* text;
		};

	private:

		SyntaxNode* MatchTokensAgainstRule(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, const Grammar::Rule* rule, const Grammar& grammar);

		// TODO: Store parse cache here.
	};
}