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
	const Grammar::Rule* rule = this->grammar->GetInitialRule();
	if (!rule)
		return nullptr;

	if (this->tokenArray->size() == 0)
		return nullptr;

	GeneralSyntaxNode* rootNode = new GeneralSyntaxNode();
	*rootNode->text = *rule->name;
	rootNode->range = Range{ 0, (signed)this->tokenArray->size() - 1 };

	if (!this->GenerateChildrenForNode(rootNode))
	{
		delete rootNode;
		rootNode = nullptr;
	}
	else
	{
		// TODO: Go assign file locations and text to all the syntax nodes simply by using the range member as an index into the token array.
	}

	return rootNode;
}

bool GeneralParseAlgorithm::GenerateChildrenForNode(GeneralSyntaxNode* parentNode)
{
	assert(parentNode->childList->size() == 0);

	const Grammar::Rule* rule = this->grammar->LookupRule(*parentNode->text);
	if (!rule)
		return true;	// The node is a terminal node.  We're done!

	for (Grammar::MatchSequence* matchSequence : *rule->matchSequenceArray)
	{
		if (this->GenerateChildrenForNodeWithMatchSequence(parentNode, matchSequence))
			break;
		else
			parentNode->WipeChildren();
	}

	return parentNode->GetChildCount() > 0;
}

bool GeneralParseAlgorithm::GenerateChildrenForNodeWithMatchSequence(GeneralSyntaxNode* parentNode, const Grammar::MatchSequence* matchSequence)
{
	assert(parentNode->childList->size() == 0);

	if (matchSequence->tokenSequence->size() == 0)
		return false;

	int i = -1;
	int j = -1;
	int delta = 0;

	switch (matchSequence->type)
	{
		case Grammar::MatchSequence::Type::LEFT_TO_RIGHT:
		{
			if (parentNode->range.min == -1)
				return false;

			i = parentNode->range.min;
			j = 0;
			delta = 1;
			break;
		}
		case Grammar::MatchSequence::Type::RIGHT_TO_LEFT:
		{
			if (parentNode->range.max == -1)
				return false;

			i = parentNode->range.max;
			j = (signed)matchSequence->tokenSequence->size() - 1;
			delta = -1;
			break;
		}
		default:
		{
			return false;
		}
	}

	for (const Grammar::Token* token : *matchSequence->tokenSequence)
	{
		GeneralSyntaxNode* childNode = new GeneralSyntaxNode();
		childNode->parentNode = parentNode;
		*childNode->text = token->GetText();
		parentNode->childList->push_back(childNode);
	}

	// TODO: Pin down the terminals, if any; then pin down the non-terminals.  Some may need to have undefined (min/max = -1) boundaries.
	//       This is so hard and I think I've lost the will to continue coding this for now.  This is a very hard problem, and I think it
	//       requires some sophisticated back-tracking.  Some boundaries have to remain unknown (min/max = -1) until they can be clarified
	//       down the road.  The general grammar problem is much harder to solve than some of its subclasses.

	return false;
}

GeneralParseAlgorithm::GeneralSyntaxNode::GeneralSyntaxNode()
{
	this->range = Range{ -1, -1 };
}

/*virtual*/ GeneralParseAlgorithm::GeneralSyntaxNode::~GeneralSyntaxNode()
{
}