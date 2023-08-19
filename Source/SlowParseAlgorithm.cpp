#include "SlowParseAlgorithm.h"

using namespace ParseParty;

SlowParseAlgorithm::SlowParseAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar) : Parser::Algorithm(tokenArray, grammar)
{
}

/*virtual*/ SlowParseAlgorithm::~SlowParseAlgorithm()
{
}

/*virtual*/ Parser::SyntaxNode* SlowParseAlgorithm::Parse()
{
	return nullptr;
}