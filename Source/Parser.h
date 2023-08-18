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

		SyntaxNode* ParseFile(const std::string& codeFile, const Grammar& grammar);
		SyntaxNode* Parse(const std::string& codeText, const Grammar& grammar);
		SyntaxNode* Parse(const std::vector<Lexer::Token*>& tokenArray, const Grammar& grammar);

		class PARSE_PARTY_API SyntaxNode
		{
		public:
			SyntaxNode();
			virtual ~SyntaxNode();

			void WipeChildren();
			void Flatten();

			SyntaxNode* parentNode;
			std::list<SyntaxNode*>* childList;
			std::string* text;
			Lexer::FileLocation fileLocation;
		};

	private:

		// A grammar may specify an algorithm to use.
		class Algorithm
		{
		public:
			Algorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar);
			virtual ~Algorithm();

			virtual SyntaxNode* Parse() = 0;

			const std::vector<Lexer::Token*>* tokenArray;
			const Grammar* grammar;
		};

		// The goal here is to provide an algorithm that, for some grammars, parses in linear time.
		// For other grammars, performance should be reasonably improved through the use of something
		// close to the concept of memoization.  Memory copies should be minimally performed, if not
		// eliminated altogether.  Any successfully parsed sub-region of the token array should get
		// cached for potential re-use if it is to be discarded during the parsing process.  Note that
		// some grammars fail to parse or fail to parse correctly, but that does not mean there doesn't
		// exist some other grammar generating the same language that will correctly parse using this
		// parsing algorithm!
		class QuickAlgorithm : public Algorithm
		{
		public:
			QuickAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar);
			virtual ~QuickAlgorithm();

			virtual SyntaxNode* Parse() override;

			SyntaxNode* MatchTokensAgainstRule(int& parsePosition, const Grammar::Rule* rule);

			struct ParseAttempt		// TODO: Make this key-able in a std::map class.
			{
				std::string ruleName;
				int parsePosition;
			};

			bool AlreadyAttemptingParse(const ParseAttempt& attempt) const;

			std::list<ParseAttempt>* parseAttemptStack;

			// TODO: Store parse cache here.
		};
	};
}