#pragma once

#include "Parser.h"

namespace ParseParty
{
	// This algorithm isn't going to scale well with the size the input, but it's the best I can come up with.
	// It makes only one assumption as additional information alongside the given token array and grammar, and
	// it is that all parenthesis, curly braces and square brackets are meant to be balanced.  Lastly, there is
	// just one constraint here that we put on the given grammar, and it is that no two non-terminals be adjacent
	// to one another in any match sequence.  Terminals may be adjacent, but not non-terminals.
	class SlowParseAlgorithm : public Parser::Algorithm
	{
	public:
		SlowParseAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar);
		virtual ~SlowParseAlgorithm();

		virtual Parser::SyntaxNode* Parse() override;

		struct Range
		{
			int min, max;

			bool operator==(const Range& range) const;
			void MakeKey(char* keyBuf, int keyBufSize) const;
			int Size() const;
			bool Contains(int i) const;
			bool IsValid() const;
		};

		struct ParseCacheKey
		{
			Range range;
			std::string ruleName;
		};

		Parser::SyntaxNode* ParseRangeAgainstRule(const Range& range, const Grammar::Rule* rule);
		Parser::SyntaxNode* ParseRangeAgainstMatchSequence(const Range& range, const Grammar::MatchSequence* matchSequence, const std::string& ruleName);

		bool ScanForTokenMatch(const Grammar::Token* grammarToken, int& tokenPosition, int delta, const Range& range);

		void ClearCache();

		typedef std::map<ParseCacheKey, Parser::SyntaxNode*> ParseCacheMap;
		ParseCacheMap* parseCacheMap;
		bool parseCacheMapEnabled;
		Lexer::FileLocation maxErrorLocation;
	};

	bool operator<(const SlowParseAlgorithm::ParseCacheKey& keyA, const SlowParseAlgorithm::ParseCacheKey& keyB);
}