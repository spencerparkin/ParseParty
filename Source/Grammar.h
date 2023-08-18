#pragma once

#include "Common.h"
#include "Lexer.h"
#include "JsonValue.h"

namespace ParseParty
{
	class PARSE_PARTY_API Grammar
	{
	public:
		Grammar();
		virtual ~Grammar();

		bool ReadFile(const std::string& grammarFile);
		bool WriteFile(const std::string& grammarFile) const;

		void Clear();

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
			TerminalToken(const std::string& givenText);
			virtual ~TerminalToken();

			virtual MatchResult Matches(const Lexer::Token& token, std::string& ruleName) const override;

			std::string* text;
		};

		class NonTerminalToken : public Token
		{
		public:
			NonTerminalToken();
			NonTerminalToken(const std::string& givenRuleName);
			virtual ~NonTerminalToken();

			virtual MatchResult Matches(const Lexer::Token& token, std::string& ruleName) const override;

			std::string* ruleName;
		};

		class Rule
		{
		public:
			Rule();
			virtual ~Rule();

			bool Read(const JsonArray* jsonRuleArray, const JsonObject* jsonRuleMap);
			bool Write(JsonArray* jsonRuleArray) const;

			void Clear();

			typedef std::vector<Token*> MatchSequence;

			std::vector<MatchSequence*>* matchSequenceArray;
			std::string* name;
		};

		typedef std::map<std::string, Rule*> RuleMap;
		RuleMap* ruleMap;

		std::string* initialRule;
		std::string* algorithmName;
	};
}