#pragma once

#include "Parser.h"

namespace ParseParty
{
	struct QuickParseAttempt
	{
		std::string ruleName;
		int parsePosition;
	};

	bool operator<(const QuickParseAttempt& attemptA, const QuickParseAttempt& attemptB);

	class QuickSyntaxNode : public Parser::SyntaxNode
	{
	public:
		QuickSyntaxNode();
		virtual ~QuickSyntaxNode();

		QuickParseAttempt* parseAttempt;
		int parseSize;
	};

	// The goal here is to provide an algorithm that, for some grammars, parses in linear time.
	// For other grammars, performance should be reasonably improved through the use of something
	// close to the concept of memoization.  Memory copies should be minimally performed, if not
	// eliminated altogether.  Any successfully parsed sub-region of the token array should get
	// cached for potential re-use if it is to be discarded during the parsing process.  Note that
	// some grammars fail to parse or fail to parse correctly, but that does not mean there doesn't
	// exist some other grammar generating the same language that will correctly parse using this
	// parsing algorithm!  Also note that there is no restriction here that there be no two adjacent
	// non-terminal tokens in the given grammar.
	class QuickParseAlgorithm : public Parser::Algorithm
	{
	public:
		QuickParseAlgorithm(const std::vector<std::shared_ptr<Lexer::Token>>* tokenArray, const Grammar* grammar);
		virtual ~QuickParseAlgorithm();

		virtual Parser::SyntaxNode* Parse() override;

		Parser::SyntaxNode* MatchTokensAgainstRule(int& parsePosition, const Grammar::Rule* rule);
		bool AlreadyAttemptingParse(const QuickParseAttempt& attempt) const;
		void ClearCache();

	private:

		std::list<QuickParseAttempt>* parseAttemptStack;
		typedef std::map<QuickParseAttempt, QuickSyntaxNode*> ParseCacheMap;
		ParseCacheMap* parseCacheMap;
		bool parseCacheEnabled;
		int maxParsePositionWithError;
	};
}