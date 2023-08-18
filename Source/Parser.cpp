#include "Parser.h"
#include "Grammar.h"

namespace ParseParty
{
	bool operator<(const Parser::ParseAttempt& attemptA, const Parser::ParseAttempt& attemptB)
	{
		if (attemptA.parsePosition == attemptB.parsePosition)
			return ::strcmp(attemptA.ruleName.c_str(), attemptB.ruleName.c_str()) < 0;

		return attemptA.parsePosition < attemptB.parsePosition;
	}
}

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
	{
		// Nodes with the following text no longer give meaning or structure to the code.
		// The structure/meaning of the code is now found in the structure of the AST.
		std::set<std::string> textSet;
		textSet.insert(";");
		textSet.insert("(");
		textSet.insert(")");
		rootNode->RemoveNodesWithText(textSet);

		// Recursive definitions in the grammar cause unnecessary structure in the AST
		// that we are trying to remove here.
		rootNode->Flatten();
	}

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
	this->parseCacheEnabled = true;
	this->parseAttemptStack = new std::list<ParseAttempt>();
	this->parseCacheMap = new ParseCacheMap();
}

/*virtual*/ Parser::QuickAlgorithm::~QuickAlgorithm()
{
	delete this->parseAttemptStack;

	this->ClearCache();

	delete this->parseCacheMap;
}

void Parser::QuickAlgorithm::ClearCache()
{
	for (std::pair<ParseAttempt, SyntaxNode*> pair : *this->parseCacheMap)
		delete pair.second;
}

/*virtual*/ Parser::SyntaxNode* Parser::QuickAlgorithm::Parse()
{
	this->ClearCache();

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
	if (parsePosition < 0 || parsePosition >= (signed)this->tokenArray->size())
		return nullptr;

	ParseAttempt parseAttempt{ *rule->name, parsePosition };

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

	for(const Grammar::Rule::MatchSequence* matchSequence : *rule->matchSequenceArray)
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
			// We didn't complete the match, but don't throw away any successful parsing that was performed if the cache is enabled.
			if (this->parseCacheEnabled)
			{
				for (std::list<SyntaxNode*>::iterator iter = parentNode->childList->begin(); iter != parentNode->childList->end(); iter++)
				{
					QuickSyntaxNode* childNode = static_cast<QuickSyntaxNode*>(*iter);
					if (childNode->parseAttempt->parsePosition >= 0)
					{
						*iter = nullptr;
						assert(this->parseCacheMap->find(*childNode->parseAttempt) == this->parseCacheMap->end());
						this->parseCacheMap->insert(std::pair<ParseAttempt, QuickSyntaxNode*>(*childNode->parseAttempt, childNode));
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

	std::list<SyntaxNode*>::iterator nextIterA;
	for (std::list<SyntaxNode*>::iterator iterA = this->childList->begin(); iterA != this->childList->end(); iterA = nextIterA)
	{
		nextIterA = iterA;
		nextIterA++;

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

void Parser::SyntaxNode::RemoveNodesWithText(const std::set<std::string>& textSet)
{
	// TODO: Isn't there some fancy boost algorithm for this kind of stuff?  Like some sort of filter function?
	std::list<SyntaxNode*>::iterator nextIter;
	for (std::list<SyntaxNode*>::iterator iter = this->childList->begin(); iter != this->childList->end(); iter = nextIter)
	{
		nextIter = iter;
		nextIter++;

		SyntaxNode* childNode = *iter;
		if (textSet.find(*childNode->text) != textSet.end())
		{
			delete childNode;
			this->childList->erase(iter);
		}
	}

	for (SyntaxNode* childNode : *this->childList)
		childNode->RemoveNodesWithText(textSet);
}

//------------------------------- Parser::QuickSyntaxNode -------------------------------

Parser::QuickSyntaxNode::QuickSyntaxNode()
{
	this->parseAttempt = new ParseAttempt;
	this->parseAttempt->parsePosition = -1;
	this->parseSize = -1;
}

/*virtual*/ Parser::QuickSyntaxNode::~QuickSyntaxNode()
{
	delete this->parseAttempt;
}