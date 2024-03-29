#pragma once

#include "Common.h"
#include "Lexer.h"
#include "JsonValue.h"

#define PARSE_PARTY_GRAMMAR_FLAG_FLATTEN_AST				0x00000001
#define PARSE_PARTY_GRAMMAR_FLAG_DELETE_STRUCTURE_TOKENS	0x00000002

namespace ParseParty
{
	class PARSE_PARTY_API Grammar
	{
	public:
		Grammar();
		virtual ~Grammar();

		bool ReadFile(const std::string& grammarFile, std::string& error);
		bool WriteFile(const std::string& grammarFile) const;

		void Clear();

		// TODO: Given a lexer, we could generate a random code sequence which should always parse correctly.

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

			virtual MatchResult Matches(const Lexer::Token& token, std::string* ruleName = nullptr) const = 0;
			virtual std::string GetText() const = 0;
		};

		class TerminalToken : public Token
		{
		public:
			TerminalToken();
			TerminalToken(const std::string& givenText);
			virtual ~TerminalToken();

			virtual MatchResult Matches(const Lexer::Token& token, std::string* ruleName = nullptr) const override;
			virtual std::string GetText() const override;

			std::string* text;
		};

		class NonTerminalToken : public Token
		{
		public:
			NonTerminalToken();
			NonTerminalToken(const std::string& givenRuleName);
			virtual ~NonTerminalToken();

			virtual MatchResult Matches(const Lexer::Token& token, std::string* ruleName = nullptr) const override;
			virtual std::string GetText() const override;

			std::string* ruleName;
		};

		class MatchSequence
		{
		public:
			MatchSequence();
			virtual ~MatchSequence();

			void Clear();
			bool HasTwoAdjacentNonTermainls() const;

			enum class Type
			{
				LEFT_TO_RIGHT,
				RIGHT_TO_LEFT
			};

			std::vector<Token*>* tokenSequence;
			Type type;
		};

		class Rule
		{
		public:
			Rule();
			virtual ~Rule();

			bool Read(const JsonArray* jsonRuleArray, const JsonObject* jsonRuleMap);
			bool Write(JsonArray* jsonRuleArray) const;

			void Clear();

			std::vector<MatchSequence*>* matchSequenceArray;
			std::string* name;
		};

		bool ReadFlags(const JsonObject* jsonFlags);

		typedef std::map<std::string, Rule*> RuleMap;
		RuleMap* ruleMap;

		std::string* initialRule;
		std::string* algorithmName;
		int flags;
	};
}