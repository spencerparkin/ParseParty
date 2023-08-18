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
	Algorithm* algorithm = nullptr;

	if (*grammar.algorithmName == "quick")
		algorithm = new QuickAlgorithm(&tokenArray, &grammar);

	if (!algorithm)
		return nullptr;

	SyntaxNode* rootNode = algorithm->Parse();

	delete algorithm;

	if (rootNode)
		rootNode->Flatten();

	return rootNode;
}

//------------------------------- Parser::Algorithm -------------------------------

Parser::Algorithm::Algorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar)
{
	this->tokenArray = tokenArray;
	this->grammar = grammar;
}

/*virtual*/ Parser::Algorithm::~Algorithm()
{
}

//------------------------------- Parser::QuickAlgorithm -------------------------------

Parser::QuickAlgorithm::QuickAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar) : Algorithm(tokenArray, grammar)
{
	this->parseAttemptStack = new std::list<ParseAttempt>();
}

/*virtual*/ Parser::QuickAlgorithm::~QuickAlgorithm()
{
	delete this->parseAttemptStack;
}

/*virtual*/ Parser::SyntaxNode* Parser::QuickAlgorithm::Parse()
{
	const Grammar::Rule* rule = this->grammar->GetInitialRule();
	if (!rule)
		return nullptr;

	int parsePosition = 0;
	return this->MatchTokensAgainstRule(parsePosition, rule);
}

// TODO: How do we report parse errors?  If a given token sequence ultimately fails to parse, then we must report an error,
//       but failures to parse are inherently part of the parsing process anyway.  So how do we know which parse failures
//       are do to syntax errors in the given code?
Parser::SyntaxNode* Parser::QuickAlgorithm::MatchTokensAgainstRule(int& parsePosition, const Grammar::Rule* rule)
{
	SyntaxNode* parentNode = new SyntaxNode();
	*parentNode->text = *rule->name;

	this->parseAttemptStack->push_back(ParseAttempt{ *rule->name, parsePosition });

	// TODO: First look in the cache.  If we have already successfully parsed the token array at the given position, against the given rule, then return the result.

	for(const Grammar::Rule::MatchSequence* matchSequence : *rule->matchSequenceArray)
	{
		int initialParsePosition = parsePosition;

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
					SyntaxNode* childNode = new SyntaxNode();
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
					if (subRule && !this->AlreadyAttemptingParse(ParseAttempt{ ruleName, parsePosition }))
					{
						SyntaxNode* childNode = this->MatchTokensAgainstRule(parsePosition, subRule);
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

bool Parser::QuickAlgorithm::AlreadyAttemptingParse(const ParseAttempt& attempt) const
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