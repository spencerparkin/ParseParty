#include "SlowParseAlgorithm.h"
#include <cstring>

namespace ParseParty
{
	bool operator<(const SlowParseAlgorithm::ParseCacheKey& keyA, const SlowParseAlgorithm::ParseCacheKey& keyB)
	{
		if (keyA.ruleName == keyB.ruleName)
		{
			char bufA[128];
			char bufB[128];

			keyA.range.MakeKey(bufA, sizeof(bufA));
			keyB.range.MakeKey(bufB, sizeof(bufB));

			return ::strcmp(bufA, bufB) < 0;
		}

		return ::strcmp(keyA.ruleName.c_str(), keyB.ruleName.c_str()) < 0;
	}
}

using namespace ParseParty;

SlowParseAlgorithm::SlowParseAlgorithm(const std::vector<Lexer::Token*>* tokenArray, const Grammar* grammar) : Parser::Algorithm(tokenArray, grammar)
{
	this->parseCacheMap = new ParseCacheMap();
	this->parseCacheMapEnabled = true;
	this->maxErrorLocation = Lexer::FileLocation{ 0, 0 };
}

/*virtual*/ SlowParseAlgorithm::~SlowParseAlgorithm()
{
	this->ClearCache();

	delete this->parseCacheMap;
}

void SlowParseAlgorithm::ClearCache()
{
	for (std::pair<ParseCacheKey, Parser::SyntaxNode*> pair : *this->parseCacheMap)
		delete pair.second;

	this->parseCacheMap->clear();
}

/*virtual*/ Parser::SyntaxNode* SlowParseAlgorithm::Parse()
{
	const Grammar::Rule* rule = this->grammar->GetInitialRule();
	if (!rule)
		return nullptr;

	this->ClearCache();

	Range range{ 0, int(this->tokenArray->size() - 1) };
	if (range.Size() <= 0)
		return nullptr;

	this->maxErrorLocation = Lexer::FileLocation{ 0, 0 };

	return this->ParseRangeAgainstRule(range, rule);
}

Parser::SyntaxNode* SlowParseAlgorithm::ParseRangeAgainstRule(const Range& range, const Grammar::Rule* rule)
{
	// The parse cache is purely an optimization, and should not be needed for correctness of the algorithm.
	if (this->parseCacheMapEnabled)
	{
		ParseCacheKey key{ range, *rule->name };
		ParseCacheMap::iterator iter = this->parseCacheMap->find(key);
		if (iter != this->parseCacheMap->end())
		{
			Parser::SyntaxNode* syntaxNode = iter->second;
			this->parseCacheMap->erase(iter);
			return syntaxNode;
		}
	}

	for (const Grammar::MatchSequence* matchSequence : *rule->matchSequenceArray)
	{
		Parser::SyntaxNode* syntaxNode = this->ParseRangeAgainstMatchSequence(range, matchSequence, *rule->name);
		if (syntaxNode)
			return syntaxNode;
	}

	return nullptr;
}

Parser::SyntaxNode* SlowParseAlgorithm::ParseRangeAgainstMatchSequence(const Range& range, const Grammar::MatchSequence* matchSequence, const std::string& ruleName)
{
	std::map<int, Range> subRangeMap;
	if (!this->CalculateSubRangeMap(subRangeMap, range, matchSequence))
		return nullptr;

	Parser::SyntaxNode* parentNode = new Parser::SyntaxNode();
	parentNode->fileLocation = (*this->tokenArray)[range.min]->fileLocation;
	*parentNode->text = ruleName;

	for (int i = 0; i < (signed)matchSequence->tokenSequence->size(); i++)
	{
		const Grammar::Token* grammarToken = (*matchSequence->tokenSequence)[i];
		
		std::map<int, Range>::iterator iter = subRangeMap.find(i);
		const Range& subRange = iter->second;

		Parser::SyntaxNode* childNode = nullptr;

		const Grammar::TerminalToken* terminalToken = dynamic_cast<const Grammar::TerminalToken*>(grammarToken);
		if (terminalToken)
		{
			Parser::SyntaxNode* dataNode = new Parser::SyntaxNode();
			dataNode->fileLocation = (*this->tokenArray)[subRange.min]->fileLocation;
			*dataNode->text = *(*this->tokenArray)[subRange.min]->text;
			childNode = new Parser::SyntaxNode();
			childNode->fileLocation = dataNode->fileLocation;
			*childNode->text = *terminalToken->text;
			childNode->childList->push_back(dataNode);
			dataNode->parentNode = childNode;
		}

		const Grammar::NonTerminalToken* nonTerminalToken = dynamic_cast<const Grammar::NonTerminalToken*>(grammarToken);
		if (nonTerminalToken)
		{
			const Grammar::Rule* rule = this->grammar->LookupRule(*nonTerminalToken->ruleName);
			if (rule)
				childNode = this->ParseRangeAgainstRule(subRange, rule);
		}

		if (!childNode)
			break;

		parentNode->childList->push_back(childNode);
		childNode->parentNode = parentNode;
	}

	if (parentNode->childList->size() != matchSequence->tokenSequence->size())
	{
		// I think this method of parse-error reporting will be accurate enough, provided that
		// parsing generally happens from left to right.  There are some cases where it needs to
		// happen right to left, but perhaps those are few enough.
		const Lexer::FileLocation& fileLocationMin = (*this->tokenArray)[range.min]->fileLocation;
		if (this->maxErrorLocation < fileLocationMin)
		{
			this->maxErrorLocation = fileLocationMin;
			const Lexer::FileLocation& fileLocationMax = (*this->tokenArray)[range.max]->fileLocation;
			if (fileLocationMin.line == fileLocationMax.line)
				*this->error = FormatString("Failed to parse line %d, columns %d to %d.", fileLocationMin.line, fileLocationMin.column, fileLocationMax.column);
			else
				*this->error = FormatString("Failed to parse from line %d (column %d) to line %d (column %d).", fileLocationMin.line, fileLocationMin.column, fileLocationMax.line, fileLocationMax.column);
		}

		if (this->parseCacheMapEnabled)
		{
			// Cache successfully parsed child nodes from the parent node before we destroy the unsuccessfully parsed parent node.
			int i = 0;
			for (std::list<Parser::SyntaxNode*>::iterator iter = parentNode->childList->begin(); iter != parentNode->childList->end(); iter++)
			{
				Parser::SyntaxNode* childNode = *iter;
				const Grammar::Token* grammarToken = (*matchSequence->tokenSequence)[i];
				const Grammar::NonTerminalToken* nonTerminalToken = dynamic_cast<const Grammar::NonTerminalToken*>(grammarToken);
				if (nonTerminalToken)
				{
					const Range& subRange = subRangeMap.find(i)->second;
					const Grammar::Rule* rule = this->grammar->LookupRule(*nonTerminalToken->ruleName);
					ParseCacheKey key{ subRange, *rule->name };
					assert(this->parseCacheMap->find(key) == this->parseCacheMap->end());
					this->parseCacheMap->insert(std::pair<ParseCacheKey, Parser::SyntaxNode*>(key, childNode));
					*iter = nullptr;
					childNode->parentNode = nullptr;
				}

				i++;
			}
		}

		delete parentNode;
		parentNode = nullptr;
	}

	return parentNode;
}

bool SlowParseAlgorithm::CalculateSubRangeMap(std::map<int, Range>& subRangeMap, const Range& range, const Grammar::MatchSequence* matchSequence)
{
	// Verify an assumption we're making here before we go bumbling forward.
	if (matchSequence->HasTwoAdjacentNonTermainls())
		return false;

	int i_start = -1, i_stop = -1, i_delta = 0;
	int tokenPosition = -1;

	switch (matchSequence->type)
	{
		case Grammar::MatchSequence::Type::LEFT_TO_RIGHT:
		{
			i_start = 0;
			i_stop = (int)matchSequence->tokenSequence->size();
			i_delta = 1;
			tokenPosition = range.min;
			break;
		}
		case Grammar::MatchSequence::Type::RIGHT_TO_LEFT:
		{
			i_start = (int)matchSequence->tokenSequence->size() - 1;
			i_stop = -1;
			i_delta = -1;
			tokenPosition = range.max;
			break;
		}
	}

	int j = -1;
	for (int i = i_start; i != i_stop; i += i_delta)
	{
		const Grammar::Token* grammarToken = (*matchSequence->tokenSequence)[i];
		const Grammar::TerminalToken* terminalToken = dynamic_cast<const Grammar::TerminalToken*>(grammarToken);
		if (!terminalToken)
			continue;

		if (!this->ScanForTokenMatch(grammarToken, tokenPosition, i_delta, range))
			return false;

		if (tokenPosition == j)
		{
			tokenPosition += i_delta;
			if (!this->ScanForTokenMatch(grammarToken, tokenPosition, i_delta, range))
				return false;
		}

		Range terminalRange{ tokenPosition, tokenPosition };
		subRangeMap.insert(std::pair<int, Range>(i, terminalRange));
		j = tokenPosition;
	}

	for (int i = 0; i < (signed)matchSequence->tokenSequence->size(); i++)
	{
		const Grammar::Token* grammarToken = (*matchSequence->tokenSequence)[i];
		const Grammar::NonTerminalToken* nonTerminalToken = dynamic_cast<const Grammar::NonTerminalToken*>(grammarToken);
		if (!nonTerminalToken)
			continue;

		Range nonTerminalRange;

		if (i == 0)
			nonTerminalRange.min = range.min;
		else
		{
			std::map<int, Range>::iterator iter = subRangeMap.find(i - 1);
			if (iter == subRangeMap.end())
				return false;

			nonTerminalRange.min = iter->second.max + 1;
		}

		if (i == matchSequence->tokenSequence->size() - 1)
			nonTerminalRange.max = range.max;
		else
		{
			std::map<int, Range>::iterator iter = subRangeMap.find(i + 1);
			if (iter == subRangeMap.end())
				return false;

			nonTerminalRange.max = iter->second.min - 1;
		}

		subRangeMap.insert(std::pair<int, Range>(i, nonTerminalRange));
	}

	int totalSize = 0;
	for (const std::pair<int, Range>& pair : subRangeMap)
	{
		const Range& subRange = pair.second;
		if (!subRange.IsValid())
			return false;

		totalSize += subRange.Size();
	}

	if (totalSize != range.Size())
		return false;

	for (int i = 0; i < (signed)matchSequence->tokenSequence->size() - 1; i++)
	{
		std::map<int, Range>::iterator iterA = subRangeMap.find(i);
		std::map<int, Range>::iterator iterB = subRangeMap.find(i + 1);

		const Range& subRangeA = iterA->second;
		const Range& subRangeB = iterB->second;

		if (subRangeA.max - subRangeB.min != -1)
			return false;
	}

	return true;
}

bool SlowParseAlgorithm::ScanForTokenMatch(const Grammar::Token* grammarToken, int& tokenPosition, int delta, const Range& range)
{
	int level = 0;

	while (range.Contains(tokenPosition))
	{
		const Lexer::Token* token = (*this->tokenArray)[tokenPosition];

		if ((delta > 0 && token->IsCloser()) || (delta < 0 && token->IsOpener()))
			level = (level > 0) ? (level - 1) : 0;

		if (level == 0)
		{
			if (grammarToken->Matches(*token) == Grammar::Token::MatchResult::YES)
				return true;
		}

		if ((delta > 0 && token->IsOpener()) || (delta < 0 && token->IsCloser()))
			level++;

		tokenPosition += delta;
	}

	return false;
}

void SlowParseAlgorithm::Range::MakeKey(char* keyBuf, int keyBufSize) const
{
	sprintf(keyBuf, "%d-%d", this->min, this->max);
}

int SlowParseAlgorithm::Range::Size() const
{
	return this->max - this->min + 1;
}

bool SlowParseAlgorithm::Range::Contains(int i) const
{
	return this->min <= i && i <= this->max;
}

bool SlowParseAlgorithm::Range::operator==(const Range& range) const
{
	return this->min == range.min && this->max == range.max;
}

bool SlowParseAlgorithm::Range::IsValid() const
{
	return this->min <= this->max;
}