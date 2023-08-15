#include "Grammar.h"

using namespace ParseParty;

//------------------------------- Grammar -------------------------------

Grammar::Grammar()
{
	this->ruleMap = new RuleMap();
	this->initialRule = new std::string();
}

/*virtual*/ Grammar::~Grammar()
{
	delete this->ruleMap;
	delete this->initialRule;
}

const Grammar::Rule* Grammar::GetInitialRule() const
{
	return this->LookupRule(*this->initialRule);
}

const Grammar::Rule* Grammar::LookupRule(const std::string& ruleName) const
{
	RuleMap::iterator iter = this->ruleMap->find(ruleName);
	if (iter == this->ruleMap->end())
		return nullptr;

	return iter->second;
}

//------------------------------- Grammar::Token -------------------------------

Grammar::Token::Token()
{
}

/*virtual*/ Grammar::Token::~Token()
{
}

//------------------------------- Grammar::TermionalToken -------------------------------

Grammar::TerminalToken::TerminalToken()
{
	this->text = new std::string();
	this->type = Lexer::Token::Type::UNKNOWN;
}

/*virtual*/ Grammar::TerminalToken::~TerminalToken()
{
	delete this->text;
}

/*virtual*/ Grammar::Token::MatchResult Grammar::TerminalToken::Matches(const Lexer::Token& token, std::string& ruleName) const
{
	switch (this->type)
	{
		case Lexer::Token::Type::STRING_LITERAL:
		case Lexer::Token::Type::NUMBER_LITERAL:
		case Lexer::Token::Type::IDENTIFIER:
		{
			return (this->type == token.type) ? MatchResult::YES : MatchResult::NO;
		}
	}

	return (this->type == token.type && *this->text == *token.text) ? MatchResult::YES : MatchResult::NO;
}

//------------------------------- Grammar::NonTerminalToken -------------------------------

Grammar::NonTerminalToken::NonTerminalToken()
{
	this->ruleName = new std::string();
}

/*virtual*/ Grammar::NonTerminalToken::~NonTerminalToken()
{
	delete this->ruleName;
}

/*virtual*/ Grammar::Token::MatchResult Grammar::NonTerminalToken::Matches(const Lexer::Token& token, std::string& ruleName) const
{
	ruleName = *this->ruleName;
	return MatchResult::MAYBE;
}

//------------------------------- Grammar::Rule -------------------------------

Grammar::Rule::Rule()
{
	this->tokenSequenceArray = nullptr;
	this->tokenSequenceSize = 0;
	this->name = new std::string();
}

/*virtual*/ Grammar::Rule::~Rule()
{
	delete this->name;

	for (int i = 0; i < (signed)this->tokenSequenceSize; i++)
		for (int j = 0; j < (signed)this->tokenSequenceArray[j].size(); j++)
			delete this->tokenSequenceArray[i][j];

	delete[] this->tokenSequenceArray;
}