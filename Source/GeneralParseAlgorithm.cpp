#include "GeneralParseAlgorithm.h"

using namespace ParseParty;

GeneralParseAlgorithm::GeneralParseAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar) : Parser::Algorithm(tokenArray, grammar)
{
}

/*virtual*/ GeneralParseAlgorithm::~GeneralParseAlgorithm()
{
}

/*virtual*/ Parser::SyntaxNode* GeneralParseAlgorithm::Parse()
{
	return nullptr;
}