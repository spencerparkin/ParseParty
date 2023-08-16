#include "Parser.h"
#include "Grammar.h"

using namespace ParseParty;

//------------------------------- Parser -------------------------------

Parser::Parser()
{
}

/*virtual*/ Parser::~Parser()
{
}

Parser::SyntaxNode* Parser::Parse(const std::vector<Lexer::Token*>& tokenArray, const Grammar& grammar)
{
	const Grammar::Rule* rule = grammar.GetInitialRule();
	if (!rule)
		return nullptr;

	int parsePosition = 0;
	SyntaxNode* rootNode = this->MatchTokensAgainstRule(tokenArray, parsePosition, rule, grammar);

	return rootNode;
}

Parser::SyntaxNode* Parser::MatchTokensAgainstRule(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, const Grammar::Rule* rule, const Grammar& grammar)
{
	SyntaxNode* parentNode = new SyntaxNode();
	*parentNode->text = *rule->name;

	// TODO: First look in the cache.  If we have already successfully parsed the token array at the given position, against the given rule, then return the result.

	for(const Grammar::MatchSequence* matchSequence : *rule->matchSequenceArray)
	{
		int initialParsePosition = parsePosition;
		bool goodMatch = false;

		// TODO: How do we adhere to the match sequence's type?  (L-to-R or R-to-L?)  I think we may need to use a parse range instead of a parse position.
		//       Also, the type will indicate which direction we're trying to match tokens.  For now, it is always, L-to-R.

		for (int j = 0; j < (signed)matchSequence->tokenSequence->size(); j++)
		{
			const Grammar::Token* grammarToken = (*matchSequence->tokenSequence)[j];
			const Lexer::Token* token = tokenArray[parsePosition];

			std::string ruleName;
			goodMatch = false;
			
			switch (grammarToken->Matches(*token, ruleName))
			{
				case Grammar::Token::MatchResult::YES:
				{
					SyntaxNode* childNode = new SyntaxNode();
					*childNode->text = *token->text;
					parentNode->childArray->push_back(childNode);
					goodMatch = true;
					parsePosition++;
					break;
				}
				case Grammar::Token::MatchResult::MAYBE:
				{
					const Grammar::Rule* subRule = grammar.LookupRule(ruleName);
					if (subRule)
					{
						SyntaxNode* childNode = this->MatchTokensAgainstRule(tokenArray, parsePosition, subRule, grammar);
						if (childNode)
						{
							parentNode->childArray->push_back(childNode);
							goodMatch = true;
						}
					}

					break;
				}
			}

			if (!goodMatch)
				break;
		}

		if (goodMatch)
			break;
		else
		{
			// TODO: Wipe the child array, but instead of deleting the child nodes, cache them for potential re-use!
			parentNode->WipeChildren();
			parsePosition = initialParsePosition;
		}
	}

	if (parentNode->childArray->size() == 0)
	{
		delete parentNode;
		parentNode = nullptr;
	}

	return parentNode;
}

//------------------------------- Parser::SyntaxNode -------------------------------

Parser::SyntaxNode::SyntaxNode()
{
	this->parentNode = nullptr;
	this->childArray = new std::vector<SyntaxNode*>();
	this->text = new std::string();
}

/*virtual*/ Parser::SyntaxNode::~SyntaxNode()
{
	this->WipeChildren();

	delete this->text;
	delete this->childArray;
}

void Parser::SyntaxNode::WipeChildren()
{
	for (SyntaxNode* childNode : *this->childArray)
		delete childNode;

	this->childArray->clear();
}