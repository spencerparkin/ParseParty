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
	// TODO: I have no idea how to write this.  Still thinking about it.

	// TODO: It's hard to research in the literature on this subject, because it's super technical and I don't understand most of it, but
	//       I wonder if the problem I'm trying to solve here is actually undecidable; meaning, no algorithm exists which could solve it
	//       in a finite amount of time.  More than likely, I'm just not smart enough to figure it out.

	return nullptr;
}