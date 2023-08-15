#pragma once

#include "Common.h"
#include "Lexer.h"

namespace ParseParty
{
	class PARSE_PARTY_API Grammar
	{
	public:
		Grammar();
		virtual ~Grammar();

		// TODO: Be able to read/write grammar from/to file.

		class Rule;

		const Rule* GetInitialRule() const;
		const Rule* LookupRule(const std::string& ruleName) const;

		class Token
		{
		public:
			Token();
			virtual ~Token();

			enum class MatchResult
			{
				YES,
				NO,
				MAYBE
			};

			virtual MatchResult Matches(const Lexer::Token& token, std::string& ruleName) const = 0;
		};

		class TerminalToken : public Token
		{
		public:
			TerminalToken();
			virtual ~TerminalToken();

			virtual MatchResult Matches(const Lexer::Token& token, std::string& ruleName) const override;

			Lexer::Token::Type type;
			std::string* text;
		};

		class NonTerminalToken : public Token
		{
		public:
			NonTerminalToken();
			virtual ~NonTerminalToken();

			virtual MatchResult Matches(const Lexer::Token& token, std::string& ruleName) const override;

			std::string* ruleName;
		};

		class Rule
		{
		public:
			Rule();
			virtual ~Rule();

			std::vector<Token*>* tokenSequenceArray;
			unsigned int tokenSequenceSize;
			std::string* name;
		};

		typedef std::map<std::string, Rule*> RuleMap;
		RuleMap* ruleMap;

		std::string* initialRule;
	};
}