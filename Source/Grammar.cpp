#include "Grammar.h"
#include "Lexer.h"

using namespace ParseParty;

//------------------------------- Grammar -------------------------------

Grammar::Grammar()
{
	this->ruleMap = new RuleMap();
	this->initialRule = new std::string();
	this->algorithmName = new std::string();
	this->flags = 0;
}

/*virtual*/ Grammar::~Grammar()
{
	this->Clear();

	delete this->ruleMap;
	delete this->initialRule;
	delete this->algorithmName;
}

void Grammar::Clear()
{
	for (std::pair<std::string, Rule*> pair : *this->ruleMap)
		delete pair.second;

	this->ruleMap->clear();
	*this->initialRule = "";
	*this->algorithmName = "";
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

bool Grammar::ReadFile(const std::string& grammarFile, std::string& error)
{
	bool success = false;
	JsonValue* jsonRootValue = nullptr;

	this->Clear();

	while (true)
	{
		std::ifstream fileStream;
		fileStream.open(grammarFile.c_str(), std::ios::in);
		if (!fileStream.is_open())
		{
			error = "Failed to open file: " + grammarFile;
			break;
		}

		std::stringstream stringStream;
		stringStream << fileStream.rdbuf();
		std::string jsonString = stringStream.str();
		std::string parseError;
		jsonRootValue = JsonValue::ParseJson(jsonString, parseError);
		if (!jsonRootValue)
		{
			error = parseError;
			break;
		}

		JsonObject* jsonObject = dynamic_cast<JsonObject*>(jsonRootValue);
		if (!jsonObject)
		{
			error = "Expected root-level JSON object.";
			break;
		}

		JsonString* jsonInitialRule = dynamic_cast<JsonString*>(jsonObject->GetValue("initial_rule"));
		if (!jsonInitialRule)
		{
			error = "No \"initial_rule\" key found or it's not a string.";
			break;
		}

		*this->initialRule = jsonInitialRule->GetValue();

		JsonString* jsonAlgorithm = dynamic_cast<JsonString*>(jsonObject->GetValue("algorithm"));
		if (!jsonAlgorithm)
		{
			error = "No \"algorithm\" key found or it's not a string.";
			break;
		}

		*this->algorithmName = jsonAlgorithm->GetValue();

		JsonObject* jsonFlags = dynamic_cast<JsonObject*>(jsonObject->GetValue("flags"));
		if (!this->ReadFlags(jsonFlags))
			break;

		JsonObject* jsonRuleMap = dynamic_cast<JsonObject*>(jsonObject->GetValue("rules"));
		if (!jsonRuleMap)
		{
			error = "No \"rules\" key found or it's not an object.";
			break;
		}

		for (std::pair<std::string, JsonValue*> pair : *jsonRuleMap)
		{
			JsonArray* jsonRuleValue = dynamic_cast<JsonArray*>(pair.second);
			if (!jsonRuleValue)
			{
				error = "Each rule entry should be an array.";
				break;
			}

			Rule* rule = new Rule();
			*rule->name = pair.first;

			if (!rule->Read(jsonRuleValue, jsonRuleMap))
			{
				error = "Failed to read rule: " + *rule->name;
				delete rule;
				break;
			}

			this->ruleMap->insert(std::pair<std::string, Rule*>(*rule->name, rule));
		}

		if (this->ruleMap->size() != jsonRuleMap->GetSize())
			break;

		success = true;
		break;
	}

	delete jsonRootValue;
	return success;
}

bool Grammar::WriteFile(const std::string& grammarFile) const
{
	return false;
}

bool Grammar::ReadFlags(const JsonObject* jsonFlags)
{
	this->flags = 0;

	if (jsonFlags)
	{
		const JsonBool* jsonFlatten = dynamic_cast<const JsonBool*>(jsonFlags->GetValue("flatten"));
		if (jsonFlatten && jsonFlatten->GetValue())
			this->flags |= PARSE_PARTY_GRAMMAR_FLAG_FLATTEN_AST;

		const JsonBool* jsonDeleteStructureTokens = dynamic_cast<const JsonBool*>(jsonFlags->GetValue("delete_structure_tokens"));
		if (jsonDeleteStructureTokens && jsonDeleteStructureTokens->GetValue())
			this->flags |= PARSE_PARTY_GRAMMAR_FLAG_DELETE_STRUCTURE_TOKENS;
	}

	return true;
}

//------------------------------- Grammar::Token -------------------------------

Grammar::Token::Token()
{
}

/*virtual*/ Grammar::Token::~Token()
{
}

//------------------------------- Grammar::TerminalToken -------------------------------

Grammar::TerminalToken::TerminalToken()
{
	this->text = new std::string();
}

Grammar::TerminalToken::TerminalToken(const std::string& givenText)
{
	this->text = new std::string();
	*this->text = givenText;
}

/*virtual*/ Grammar::TerminalToken::~TerminalToken()
{
	delete this->text;
}

/*virtual*/ Grammar::Token::MatchResult Grammar::TerminalToken::Matches(const Lexer::Token& token, std::string* ruleName /*= nullptr*/) const
{
	if (*this->text == "@string" && token.type == Lexer::Token::Type::STRING_LITERAL)
		return MatchResult::YES;
	
	if (*this->text == "@number" && (token.type == Lexer::Token::Type::NUMBER_LITERAL_INT || token.type == Lexer::Token::Type::NUMBER_LITERAL_FLOAT))
		return MatchResult::YES;

	if (*this->text == "@int" && token.type == Lexer::Token::Type::NUMBER_LITERAL_INT)
		return MatchResult::YES;

	if (*this->text == "@float" && token.type == Lexer::Token::Type::NUMBER_LITERAL_FLOAT)
		return MatchResult::YES;

	if (*this->text == "@identifier")
		return MatchResult::YES;

	return (*this->text == *token.text) ? MatchResult::YES : MatchResult::NO;
}

/*virtual*/ std::string Grammar::TerminalToken::GetText() const
{
	return *this->text;
}

//------------------------------- Grammar::NonTerminalToken -------------------------------

Grammar::NonTerminalToken::NonTerminalToken()
{
	this->ruleName = new std::string();
}

Grammar::NonTerminalToken::NonTerminalToken(const std::string& givenRuleName)
{
	this->ruleName = new std::string();
	*this->ruleName = givenRuleName;
}

/*virtual*/ Grammar::NonTerminalToken::~NonTerminalToken()
{
	delete this->ruleName;
}

/*virtual*/ Grammar::Token::MatchResult Grammar::NonTerminalToken::Matches(const Lexer::Token& token, std::string* ruleName /*= nullptr*/) const
{
	if (ruleName)
		*ruleName = *this->ruleName;

	return MatchResult::MAYBE;
}

/*virtual*/ std::string Grammar::NonTerminalToken::GetText() const
{
	return *this->ruleName;
}

//------------------------------- Grammar::Rule -------------------------------

Grammar::Rule::Rule()
{
	this->matchSequenceArray = new std::vector<MatchSequence*>();
	this->name = new std::string();
}

/*virtual*/ Grammar::Rule::~Rule()
{
	this->Clear();

	delete this->name;
	delete this->matchSequenceArray;
}

void Grammar::Rule::Clear()
{
	for (MatchSequence* matchSequence : *this->matchSequenceArray)
		delete matchSequence;

	this->matchSequenceArray->clear();
}

bool Grammar::Rule::Read(const JsonArray* jsonRuleArray, const JsonObject* jsonRuleMap)
{
	if (jsonRuleArray->GetSize() == 0)
		return false;

	this->Clear();

	for (int i = 0; i < (signed)jsonRuleArray->GetSize(); i++)
	{
		MatchSequence* matchSequence = new MatchSequence();
		this->matchSequenceArray->push_back(matchSequence);

		const JsonArray* jsonRuleSequence = dynamic_cast<const JsonArray*>(jsonRuleArray->GetValue(i));
		if (!jsonRuleSequence)
			return false;

		for (int j = 0; j < (signed)jsonRuleSequence->GetSize(); j++)
		{
			if (j == jsonRuleSequence->GetSize() - 1)
			{
				const JsonInt* jsonInt = dynamic_cast<const JsonInt*>(jsonRuleSequence->GetValue(j));
				if (jsonInt && jsonInt->GetValue() == -1)
				{
					matchSequence->type = MatchSequence::Type::RIGHT_TO_LEFT;
					break;
				}
			}

			const JsonString* jsonToken = dynamic_cast<const JsonString*>(jsonRuleSequence->GetValue(j));
			if (!jsonToken)
				return false;

			Token* token = nullptr;

			if (jsonRuleMap->GetValue(jsonToken->GetValue()))
				token = new NonTerminalToken(jsonToken->GetValue());
			else
				token = new TerminalToken(jsonToken->GetValue());

			matchSequence->tokenSequence->push_back(token);
		}
	}

	return true;
}

bool Grammar::Rule::Write(JsonArray* jsonRuleArray) const
{
	return false;
}

//------------------------------- Grammar::MatchSequence -------------------------------

Grammar::MatchSequence::MatchSequence()
{
	this->tokenSequence = new std::vector<Token*>();
	this->type = Type::LEFT_TO_RIGHT;
}

/*virtual*/ Grammar::MatchSequence::~MatchSequence()
{
	this->Clear();

	delete this->tokenSequence;
}

void Grammar::MatchSequence::Clear()
{
	for (Token* token : *this->tokenSequence)
		delete token;

	this->tokenSequence->clear();
}

bool Grammar::MatchSequence::HasTwoAdjacentNonTermainls() const
{
	for (int i = 0; i < (signed)this->tokenSequence->size() - 1; i++)
	{
		const Token* tokenA = (*this->tokenSequence)[i];
		const Token* tokenB = (*this->tokenSequence)[i + 1];

		if (dynamic_cast<const NonTerminalToken*>(tokenA) && dynamic_cast<const NonTerminalToken*>(tokenB))
			return true;
	}

	return false;
}