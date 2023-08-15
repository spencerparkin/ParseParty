#include "Lexer.h"

using namespace ParseParty;

//------------------------------- Lexer -------------------------------

Lexer::Lexer()
{
}

/*virtual*/ Lexer::~Lexer()
{
}

bool Lexer::Tokenize(const char* codeBuffer, std::vector<Token>& tokenArray)
{
	return false;
}

//------------------------------- Lexer::Token -------------------------------

Lexer::Token::Token()
{
	this->text = new std::string();
	this->type = Type::UNKNOWN;
}

/*virtual*/ Lexer::Token::~Token()
{
	delete this->text;
}