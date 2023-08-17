#include "Parser.h"
#include "Grammar.h"

using namespace ParseParty;

//------------------------------- Parser -------------------------------

Parser::Parser()
{
	this->parseAttemptStack = new std::list<ParseAttempt>();
}

/*virtual*/ Parser::~Parser()
{
	delete this->parseAttemptStack;
}

Parser::SyntaxNode* Parser::ParseFile(const std::string& codeFile, const Grammar& grammar)
{
	std::fstream fileStream(codeFile.c_str(), std::ios::in);
	if (!fileStream.is_open())
		return nullptr;

	std::stringstream stringStream;
	stringStream << fileStream.rdbuf();
	std::string codeText = stringStream.str();
	SyntaxNode* rootNode = this->Parse(codeText, grammar);
	fileStream.close();
	return rootNode;
}

Parser::SyntaxNode* Parser::Parse(const std::string& codeText, const Grammar& grammar)
{
	std::vector<Lexer::Token*> tokenArray;

	Lexer lexer;
	if (!lexer.Tokenize(codeText, tokenArray))
		return nullptr;

	SyntaxNode* rootNode = this->Parse(tokenArray, grammar);

	for (Lexer::Token* token : tokenArray)
		delete token;

	return rootNode;
}

Parser::SyntaxNode* Parser::Parse(const std::vector<Lexer::Token*>& tokenArray, const Grammar& grammar)
{
	const Grammar::Rule* rule = grammar.GetInitialRule();
	if (!rule)
		return nullptr;

	// TODO: Factory-create a parse algorithm object here, then delegate parsing to that thing.

	int parsePosition = 0;
	SyntaxNode* rootNode = this->MatchTokensAgainstRule(tokenArray, parsePosition, rule, grammar);

	rootNode->Flatten();

	return rootNode;
}

// TODO: This algorithm is just plain wrong, and I really don't know the answer.  That's what makes this a really interesting problem.
//       Correction: This algorithm is correct, but only for certain grammars; probably only those that can be parsed in linear time.
//       For example, I have been able to use it to parse JSON correctly, but it doesn't work for code that is more like program code.
//       Not quite sure yet how to definite exactly what constraints are needed on grammars for this algorithm to work.
//       It has something to do with a leading token almost always being a terminal token.
//       One constraint I don't know how to loosen is that of never allowing two non-terminals to be adjacent in any rule.
Parser::SyntaxNode* Parser::MatchTokensAgainstRule(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, const Grammar::Rule* rule, const Grammar& grammar)
{
	SyntaxNode* parentNode = new SyntaxNode();
	*parentNode->text = *rule->name;

	this->parseAttemptStack->push_back(ParseAttempt{ *rule->name, parsePosition });

	// TODO: First look in the cache.  If we have already successfully parsed the token array at the given position, against the given rule, then return the result.

	for(const Grammar::Rule::MatchSequence* matchSequence : *rule->matchSequenceArray)
	{
		int initialParsePosition = parsePosition;
		bool goodMatch = false;

		for (int j = 0; j < (signed)matchSequence->size(); j++)
		{
			const Grammar::Token* grammarToken = (*matchSequence)[j];
			const Lexer::Token* token = tokenArray[parsePosition];

			std::string ruleName;
			goodMatch = false;
			
			switch (grammarToken->Matches(*token, ruleName))
			{
				case Grammar::Token::MatchResult::YES:
				{
					SyntaxNode* childNode = new SyntaxNode();
					*childNode->text = *token->text;
					childNode->fileLocation = token->fileLocation;
					parentNode->childList->push_back(childNode);
					childNode->parentNode = parentNode;
					goodMatch = true;
					parsePosition++;
					break;
				}
				case Grammar::Token::MatchResult::MAYBE:
				{
					const Grammar::Rule* subRule = grammar.LookupRule(ruleName);

					// If we're already attempting to parse the rule at this position, then we'll infinitely recurse.
					if (subRule && !this->AlreadyAttemptingParse(ParseAttempt{ ruleName, parsePosition }))
					{
						SyntaxNode* childNode = this->MatchTokensAgainstRule(tokenArray, parsePosition, subRule, grammar);
						if (childNode)
						{
							parentNode->childList->push_back(childNode);
							childNode->parentNode = parentNode;
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

	if (parentNode->childList->size() == 0)
	{
		delete parentNode;
		parentNode = nullptr;
	}

	this->parseAttemptStack->pop_back();

	return parentNode;
}

bool Parser::AlreadyAttemptingParse(const ParseAttempt& attempt) const
{
	for (const ParseAttempt& existingAttempt : *this->parseAttemptStack)
		if (existingAttempt.ruleName == attempt.ruleName && existingAttempt.parsePosition == attempt.parsePosition)
			return true;

	return false;
}

//------------------------------- Parser::SyntaxNode -------------------------------

Parser::SyntaxNode::SyntaxNode()
{
	this->parentNode = nullptr;
	this->childList = new std::list<SyntaxNode*>();
	this->text = new std::string();
	this->fileLocation.line = -1;
	this->fileLocation.column = -1;
}

/*virtual*/ Parser::SyntaxNode::~SyntaxNode()
{
	this->WipeChildren();

	delete this->text;
	delete this->childList;
}

void Parser::SyntaxNode::WipeChildren()
{
	for (SyntaxNode* childNode : *this->childList)
		delete childNode;

	this->childList->clear();
}

void Parser::SyntaxNode::Flatten()
{
	for (SyntaxNode* childNode : *this->childList)
		childNode->Flatten();

	std::list<SyntaxNode*>::iterator iterA_next;
	for (std::list<SyntaxNode*>::iterator iterA = this->childList->begin(); iterA != this->childList->end(); iterA = iterA_next)
	{
		iterA_next = iterA;
		iterA_next++;

		SyntaxNode* childNode = *iterA;

		if (*this->text == *childNode->text)
		{
			for (std::list<SyntaxNode*>::iterator iterB = childNode->childList->begin(); iterB != childNode->childList->end(); iterB++)
				this->childList->insert(iterA, *iterB);

			childNode->childList->clear();
			delete childNode;
			this->childList->erase(iterA);
			break;
		}
	}
}