#include "QuickParseAlgorithm.h"

namespace ParseParty
{
	bool operator<(const QuickParseAttempt& attemptA, const QuickParseAttempt& attemptB)
	{
		if (attemptA.parsePosition == attemptB.parsePosition)
			return ::strcmp(attemptA.ruleName.c_str(), attemptB.ruleName.c_str()) < 0;

		return attemptA.parsePosition < attemptB.parsePosition;
	}
}

using namespace ParseParty;

//------------------------------- QuickParseAlgorithm -------------------------------

QuickParseAlgorithm::QuickParseAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar) : Algorithm(tokenArray, grammar)
{
	this->parseCacheEnabled = true;
	this->parseAttemptStack = new std::list<QuickParseAttempt>();
	this->parseCacheMap = new ParseCacheMap();
	this->maxParsePositionWithError = -1;
}

/*virtual*/ QuickParseAlgorithm::~QuickParseAlgorithm()
{
	delete this->parseAttemptStack;

	this->ClearCache();

	delete this->parseCacheMap;
}

void QuickParseAlgorithm::ClearCache()
{
	for (std::pair<QuickParseAttempt, QuickSyntaxNode*> pair : *this->parseCacheMap)
		delete pair.second;
}

/*virtual*/ Parser::SyntaxNode* QuickParseAlgorithm::Parse()
{
	this->ClearCache();

	const Grammar::Rule* rule = this->grammar->GetInitialRule();
	if (!rule)
		return nullptr;

	this->maxParsePositionWithError = -1;
	int parsePosition = 0;
	return this->MatchTokensAgainstRule(parsePosition, rule);
}

// TODO: How do we report parse errors?  If a given token sequence ultimately fails to parse, then we must report an error,
//       but failures to parse are inherently part of the parsing process anyway.  So how do we know which parse failures
//       are do to syntax errors in the given code?
Parser::SyntaxNode* QuickParseAlgorithm::MatchTokensAgainstRule(int& parsePosition, const Grammar::Rule* rule)
{
	if (parsePosition < 0 || parsePosition >= (signed)this->tokenArray->size())
		return nullptr;

	QuickParseAttempt parseAttempt{ *rule->name, parsePosition };

	// The parse cache is purely an optimization and is not needed for correctness of the algorithm.
	if (this->parseCacheEnabled)
	{
		ParseCacheMap::iterator iter = this->parseCacheMap->find(parseAttempt);
		if (iter != this->parseCacheMap->end())
		{
			QuickSyntaxNode* syntaxNode = iter->second;
			this->parseCacheMap->erase(iter);
			parsePosition += syntaxNode->parseSize;
			return syntaxNode;
		}
	}

	QuickSyntaxNode* parentNode = new QuickSyntaxNode();
	*parentNode->text = *rule->name;
	*parentNode->parseAttempt = parseAttempt;
	parentNode->fileLocation = (*this->tokenArray)[parsePosition]->fileLocation;

	this->parseAttemptStack->push_back(parseAttempt);

	int initialParsePosition = parsePosition;

	for (const Grammar::Rule::MatchSequence* matchSequence : *rule->matchSequenceArray)
	{
		int i;
		for (i = 0; i < (signed)matchSequence->size(); i++)
		{
			const Grammar::Token* grammarToken = (*matchSequence)[i];

			if (parsePosition >= (signed)this->tokenArray->size())
				break;

			const Lexer::Token* token = (*this->tokenArray)[parsePosition];

			std::string ruleName;
			bool tokenMatched = false;

			switch (grammarToken->Matches(*token, ruleName))
			{
				case Grammar::Token::MatchResult::YES:
				{
					QuickSyntaxNode* childNode = new QuickSyntaxNode();
					*childNode->text = *token->text;
					childNode->fileLocation = token->fileLocation;
					parentNode->childList->push_back(childNode);
					childNode->parentNode = parentNode;
					tokenMatched = true;
					parsePosition++;
					break;
				}
				case Grammar::Token::MatchResult::MAYBE:
				{
					const Grammar::Rule* subRule = this->grammar->LookupRule(ruleName);

					// If we're already attempting to parse the rule at this position, then we'll infinitely recurse.
					if (subRule && !this->AlreadyAttemptingParse(QuickParseAttempt{ ruleName, parsePosition }))
					{
						Parser::SyntaxNode* childNode = this->MatchTokensAgainstRule(parsePosition, subRule);
						if (childNode)
						{
							parentNode->childList->push_back(childNode);
							childNode->parentNode = parentNode;
							tokenMatched = true;
						}
					}

					break;
				}
			}

			if (!tokenMatched)
				break;
		}

		// Did we complete the match?
		if (i == matchSequence->size())
			break;
		else
		{
			// We didn't complete the match, but don't throw away any successful parsing that was performed if the cache is enabled.
			if (this->parseCacheEnabled)
			{
				for (std::list<Parser::SyntaxNode*>::iterator iter = parentNode->childList->begin(); iter != parentNode->childList->end(); iter++)
				{
					QuickSyntaxNode* childNode = static_cast<QuickSyntaxNode*>(*iter);
					if (childNode->parseAttempt->parsePosition >= 0)
					{
						*iter = nullptr;
						assert(this->parseCacheMap->find(*childNode->parseAttempt) == this->parseCacheMap->end());
						this->parseCacheMap->insert(std::pair<QuickParseAttempt, QuickSyntaxNode*>(*childNode->parseAttempt, childNode));
					}
				}
			}

			parentNode->WipeChildren();
			parsePosition = initialParsePosition;
		}
	}

	if (parentNode->childList->size() > 0)
		parentNode->parseSize = parsePosition - initialParsePosition;
	else
	{
		delete parentNode;
		parentNode = nullptr;

		// Failing to apply a rule is just a normal part of parsing, but when does is indicate an actual parsing error?
		// I'm not sure, but maybe it's helpful to know which rule failed furthest along the token sequence?
		if (parsePosition > this->maxParsePositionWithError)
		{
			this->maxParsePositionWithError = parsePosition;
			const Lexer::Token* token = (*this->tokenArray)[parsePosition];
			*this->error = std::format("Failed to parse rule \"{}\" at line {}, column {}.", rule->name->c_str(), token->fileLocation.line, token->fileLocation.column);
		}
	}

	this->parseAttemptStack->pop_back();

	return parentNode;
}

bool QuickParseAlgorithm::AlreadyAttemptingParse(const QuickParseAttempt& attempt) const
{
	for (const QuickParseAttempt& existingAttempt : *this->parseAttemptStack)
		if (existingAttempt.ruleName == attempt.ruleName && existingAttempt.parsePosition == attempt.parsePosition)
			return true;

	return false;
}

//------------------------------- QuickSyntaxNode -------------------------------

QuickSyntaxNode::QuickSyntaxNode()
{
	this->parseAttempt = new QuickParseAttempt;
	this->parseAttempt->parsePosition = -1;
	this->parseSize = -1;
}

/*virtual*/ QuickSyntaxNode::~QuickSyntaxNode()
{
	delete this->parseAttempt;
}