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
	const Grammar::Rule* rule = grammar.GetInitialRule();
	if (!rule)
		return nullptr;

	int parsePosition = 0;
	SyntaxNode* rootNode = this->MatchTokensAgainstRule(tokenArray, parsePosition, rule, grammar);

	return rootNode;
}

// TODO: This algorithm is just plain wrong, and I really don't know the answer.  That's what makes this a really interesting problem.
Parser::SyntaxNode* Parser::MatchTokensAgainstRule(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, const Grammar::Rule* rule, const Grammar& grammar)
{
	SyntaxNode* parentNode = new SyntaxNode();
	*parentNode->text = *rule->name;

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
					parentNode->childArray->push_back(childNode);
					goodMatch = true;
					parsePosition++;
					break;
				}
				case Grammar::Token::MatchResult::MAYBE:
				{
					const Grammar::Rule* subRule = grammar.LookupRule(ruleName);
					if (subRule && (*subRule->name != *rule->name || parsePosition != initialParsePosition))
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
	this->fileLocation.line = -1;
	this->fileLocation.column = -1;
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