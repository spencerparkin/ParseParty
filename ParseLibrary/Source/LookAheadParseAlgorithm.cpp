#include "LookAheadParseAlgorithm.h"

using namespace ParseParty;

LookAheadParseAlgorithm::LookAheadParseAlgorithm(const std::vector<std::shared_ptr<Lexer::Token>>* tokenArray, const Grammar* grammar) : Algorithm(tokenArray, grammar)
{
	this->lookAheadCount = 5;
	this->maxRecursionDepth = 16;
}

/*virtual*/ LookAheadParseAlgorithm::~LookAheadParseAlgorithm()
{
}

// TODO: Test this algorithm.
/*virtual*/ Parser::SyntaxNode* LookAheadParseAlgorithm::Parse()
{
	const Grammar::Rule* grammarRule = this->grammar->GetInitialRule();
	if (!grammarRule)
		return nullptr;

	int parsePosition = 0;
	return this->GenerateTree(grammarRule, parsePosition);
}

Parser::SyntaxNode* LookAheadParseAlgorithm::GenerateTree(const Grammar::Rule* grammarRule, int& parsePosition)
{
	// This is the "look ahead" part of the algorithm.
	int i = this->DetermineCorrectMatchSequence(grammarRule, parsePosition, 0);
	if (i < 0)
		return nullptr;
	
	Grammar::MatchSequence* matchSequence = (*grammarRule->matchSequenceArray)[i];

	Parser::SyntaxNode* parentNode = new Parser::SyntaxNode(*grammarRule->name, (*this->tokenArray)[parsePosition]->fileLocation);

	for (Grammar::Token* grammarToken : *matchSequence->tokenSequence)
	{
		const Lexer::Token* token = (*this->tokenArray)[parsePosition].get();

		std::string grammarRuleName;
		bool tokenMatched = false;

		switch (grammarToken->Matches(*token, &grammarRuleName))
		{
			case Grammar::Token::MatchResult::YES:
			{
				Parser::SyntaxNode* childNode = new Parser::SyntaxNode();
				*childNode->text = *token->text;
				childNode->fileLocation = token->fileLocation;
				parentNode->childList->push_back(childNode);
				childNode->parentNode = parentNode;
				parsePosition++;
				tokenMatched = true;
				break;
			}
			case Grammar::Token::MatchResult::MAYBE:
			{
				const Grammar::Rule* subRule = this->grammar->LookupRule(grammarRuleName);
				assert(subRule);
				Parser::SyntaxNode* childNode = this->GenerateTree(subRule, parsePosition);
				if (childNode)
				{
					parentNode->childList->push_back(childNode);
					childNode->parentNode = parentNode;
					tokenMatched = true;
				}
				break;
			}
            default:
            {
                break;
            }
		}

		if (!tokenMatched)
		{
			delete parentNode;
			parentNode = nullptr;
			break;
		}
	}

	return parentNode;
}

int LookAheadParseAlgorithm::DetermineCorrectMatchSequence(const Grammar::Rule* grammarRule, int parsePosition, int lookAheadPosition, int recursionDepth /*= 0*/)
{
	if (recursionDepth > this->maxRecursionDepth)
		return -1;

	for (int i = 0; i < (signed)grammarRule->matchSequenceArray->size(); i++)
	{
		Grammar::MatchSequence* matchSequence = (*grammarRule->matchSequenceArray)[i];
		if (this->TryMatchSequence(matchSequence, parsePosition, lookAheadPosition, recursionDepth + 1))
			return i;
	}

	return -1;
}

bool LookAheadParseAlgorithm::TryMatchSequence(const Grammar::MatchSequence* matchSequence, int parsePosition, int lookAheadPosition, int recursionDepth)
{
	for (int i = 0; i < (signed)matchSequence->tokenSequence->size(); i++)
	{
		int matchPosition = parsePosition + lookAheadPosition;
		if (lookAheadPosition == this->lookAheadCount || matchPosition == (signed)this->tokenArray->size())
			return true;

		const Lexer::Token* token = (*this->tokenArray)[matchPosition].get();
		const Grammar::Token* grammarToken = (*matchSequence->tokenSequence)[i];
		std::string ruleName;

		switch (grammarToken->Matches(*token, &ruleName))
		{
			case Grammar::Token::MatchResult::YES:
			{
				lookAheadPosition++;
				break;
			}
			case Grammar::Token::MatchResult::MAYBE:
			{
				const Grammar::Rule* grammarRule = this->grammar->LookupRule(ruleName);
				assert(grammarRule);

				int j = this->DetermineCorrectMatchSequence(grammarRule, parsePosition, lookAheadPosition, recursionDepth + 1);
				if (j < 0)
					return false;

				break;
			}
            default:
            {
                break;
            }
		}
	}

	return true;
}