#include "Parser.h"
#include "QuickParseAlgorithm.h"
#include "SlowParseAlgorithm.h"

using namespace ParseParty;

//------------------------------- Parser -------------------------------

Parser::Parser()
{
}

/*virtual*/ Parser::~Parser()
{
}

Parser::SyntaxNode* Parser::ParseFile(const std::string& codeFile, const Grammar& grammar, std::string* error /*= nullptr*/)
{
	std::fstream fileStream(codeFile.c_str(), std::ios::in);
	if (!fileStream.is_open())
		return nullptr;

	std::stringstream stringStream;
	stringStream << fileStream.rdbuf();
	std::string codeText = stringStream.str();
	SyntaxNode* rootNode = this->Parse(codeText, grammar, error);
	fileStream.close();
	return rootNode;
}

Parser::SyntaxNode* Parser::Parse(const std::string& codeText, const Grammar& grammar, std::string* error /*= nullptr*/)
{
	std::vector<Lexer::Token*> tokenArray;

	Lexer lexer;
	if (!lexer.Tokenize(codeText, tokenArray))
		return nullptr;

	SyntaxNode* rootNode = this->Parse(tokenArray, grammar, error);

	for (Lexer::Token* token : tokenArray)
		delete token;

	return rootNode;
}

Parser::SyntaxNode* Parser::Parse(const std::vector<Lexer::Token*>& tokenArray, const Grammar& grammar, std::string* error /*= nullptr*/)
{
	Algorithm* algorithm = nullptr;

	if (*grammar.algorithmName == "quick")
		algorithm = new QuickParseAlgorithm(&tokenArray, &grammar);
	else if (*grammar.algorithmName == "slow")
		algorithm = new SlowParseAlgorithm(&tokenArray, &grammar);

	if (!algorithm)
		return nullptr;

	SyntaxNode* rootNode = algorithm->Parse();

	if (!rootNode)
	{
		if (error)
			*error = *algorithm->error;
	}
	else
	{
		// Nodes with the following text no longer give meaning or structure to the code.
		// The structure/meaning of the code is now found in the structure of the AST.
		// TODO: Maybe what we remove here should be specified in the grammar file?
		std::set<std::string> textSet;
		textSet.insert(";");
		textSet.insert(",");
		textSet.insert("(");
		textSet.insert(")");
		textSet.insert("{");
		textSet.insert("}");
		textSet.insert("[");
		textSet.insert("]");
		rootNode->RemoveNodesWithText(textSet);

		// Recursive definitions in the grammar cause unnecessary structure in the AST
		// that we are trying to remove here.
		rootNode->Flatten();
	}

	delete algorithm;

	return rootNode;
}

//------------------------------- Parser::Algorithm -------------------------------

Parser::Algorithm::Algorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar)
{
	this->tokenArray = tokenArray;
	this->grammar = grammar;
	this->error = new std::string();
}

/*virtual*/ Parser::Algorithm::~Algorithm()
{
	delete this->error;
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

int Parser::SyntaxNode::CalcSize() const
{
	int count = 1;

	for (SyntaxNode* childNode : *this->childList)
		count += childNode->CalcSize();

	return count;
}