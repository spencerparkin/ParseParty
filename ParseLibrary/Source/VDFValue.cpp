#include "VDFValue.h"

using namespace ParseParty;

//-------------------------------- VDFValue --------------------------------

VDFValue::VDFValue()
{
}

/*virtual*/ VDFValue::~VDFValue()
{
}

/*static*/ std::shared_ptr<VDFValue> VDFValue::ParseVDF(const std::string& vdfString, std::string& parseError)
{
	Lexer lexer;
	lexer.tokenGeneratorList->push_back(new Lexer::ParanTokenGenerator());
	lexer.tokenGeneratorList->push_back(new Lexer::StringTokenGenerator());

	std::vector<Lexer::Token*> tokenArray;
	if (!lexer.Tokenize(vdfString, tokenArray, parseError))
		return std::shared_ptr<VDFValue>();

	if (tokenArray.size() == 0)
	{
		parseError = "Token sequence is size zero.";
		return std::shared_ptr<VDFValue>();
	}

	std::shared_ptr<VDFValue> vdfValue = std::make_shared<VDFBlockValue>();
	int parsePosition = 0;
	if (!vdfValue->ParseTokens(tokenArray, parsePosition, parseError))
		vdfValue.reset();

	for (Lexer::Token* token : tokenArray)
		delete token;

	return vdfValue;
}

/*static*/ std::string VDFValue::MakeError(const std::vector<Lexer::Token*>& tokenArray, int parsePosition, const std::string& errorMsg)
{
	std::string errorPrefix = "Error: ";

	if (0 <= parsePosition && parsePosition < (signed)tokenArray.size())
	{
		const Lexer::FileLocation& fileLocation = tokenArray[parsePosition]->fileLocation;
		errorPrefix += FormatString("Line %d, column %d: ", fileLocation.line, fileLocation.column);
	}

	return errorPrefix + errorMsg;
}

/*static*/ std::string VDFValue::MakeTabs(int tabCount)
{
	std::string tabString;
	for (int i = 0; i < tabCount; i++)
		tabString += "\t";
	return tabString;
}

//-------------------------------- VDFStringValue --------------------------------

VDFStringValue::VDFStringValue()
{
	this->value = new std::string();
}

/*virtual*/ VDFStringValue::~VDFStringValue()
{
	delete this->value;
}

/*virtual*/ void VDFStringValue::PrintVDF(std::string& vdfString, int tabLevel /*= 0*/) const
{
	vdfString += " \"" + *this->value + "\"\n";
}

/*virtual*/ bool VDFStringValue::ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError)
{
	if (parsePosition < 0 || parsePosition >= (signed)tokenArray.size())
	{
		parseError = "Internal error!";
		return false;
	}

	const Lexer::Token* token = tokenArray[parsePosition];

	if (token->type != Lexer::Token::Type::STRING_LITERAL)
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected string literal for value.");
		return false;
	}

	*this->value = *token->text;
	parsePosition++;

	return true;
}

void VDFStringValue::SetValue(const std::string& value)
{
	*this->value = value;
}

const std::string& VDFStringValue::GetValue() const
{
	return *this->value;
}

//-------------------------------- VDFBlockValue --------------------------------

VDFBlockValue::VDFBlockValue()
{
}

/*virtual*/ VDFBlockValue::~VDFBlockValue()
{
	this->pairArray.clear();
}

/*virtual*/ void VDFBlockValue::PrintVDF(std::string& vdfString, int tabLevel /*= 0*/) const
{
	if (tabLevel == 0)
	{
		for (const Pair& pair : this->pairArray)
		{
			vdfString += "\"" + pair.key + "\"";
			pair.value->PrintVDF(vdfString, tabLevel + 1);
		}
	}
	else
	{
		vdfString += "\n" + MakeTabs(tabLevel - 1) + "{\n";

		for (const Pair& pair : this->pairArray)
		{
			vdfString += MakeTabs(tabLevel) + "\"" + pair.key + "\"";
			pair.value->PrintVDF(vdfString, tabLevel + 1);
		}

		vdfString += MakeTabs(tabLevel - 1) + "}\n";
	}
}

/*virtual*/ bool VDFBlockValue::ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError)
{
	if (parsePosition < 0)
	{
		parseError = "Internal error!";
		return false;
	}

	bool mustFindCloseCurly = false;
	bool foundCloseCurly = false;

	while (parsePosition < (signed)tokenArray.size())
	{
		const Lexer::Token* token = tokenArray[parsePosition];

		if (token->type == Lexer::Token::Type::OPEN_CURLY_BRACE)
		{
			parsePosition++;
			mustFindCloseCurly = true;
			continue;
		}

		if (token->type == Lexer::Token::Type::CLOSE_CURLY_BRACE)
		{
			parsePosition++;
			foundCloseCurly = true;
			break;
		}

		if (token->type != Lexer::Token::Type::STRING_LITERAL)
		{
			parseError = MakeError(tokenArray, parsePosition, "Expected string literal for key.");
			return false;
		}

		Pair pair;
		pair.key = *token->text;

		if (++parsePosition >= (signed)tokenArray.size())
		{
			parseError = MakeError(tokenArray, parsePosition, "Expected value for key but hit unexpected end of token sequence instead.");
			return false;
		}

		token = tokenArray[parsePosition];

		if (token->type == Lexer::Token::Type::STRING_LITERAL)
		{
			auto vdfStringValue = std::make_shared<VDFStringValue>();
			if (!vdfStringValue->ParseTokens(tokenArray, parsePosition, parseError))
				return false;

			pair.value = vdfStringValue;
		}
		else if (token->type == Lexer::Token::Type::OPEN_CURLY_BRACE)
		{
			auto vdfBlockValue = std::make_shared<VDFBlockValue>();
			if (!vdfBlockValue->ParseTokens(tokenArray, parsePosition, parseError))
				return false;

			pair.value = vdfBlockValue;
		}
		else
		{
			parseError = MakeError(tokenArray, parsePosition, "Expected string literal or block opener.");
			return false;
		}

		this->pairArray.push_back(pair);
	}

	if (mustFindCloseCurly && !foundCloseCurly)
	{
		parseError = "Run-away curly detected!";
		return false;
	}

	return true;
}

const std::shared_ptr<VDFValue> VDFBlockValue::FindValue(const std::string& key, int keyInstance /*= 0*/) const
{
	return const_cast<VDFBlockValue*>(this)->FindValue(key, keyInstance);
}

std::shared_ptr<VDFValue> VDFBlockValue::FindValue(const std::string& key, int keyInstance /*= 0*/)
{
	for (Pair& pair : this->pairArray)
	{
		if (pair.key == key)
		{
			if (keyInstance <= 0)
				return pair.value;

			keyInstance--;
		}
	}

	return nullptr;
}

bool VDFBlockValue::DeleteKey(const std::string& key, int keyInstance /*= 0*/)
{
	for (int i = 0; i < (int)this->pairArray.size(); i++)
	{
		Pair& pair = this->pairArray[i];

		if (pair.key == key)
		{
			if (keyInstance <= 0)
			{
				this->pairArray.erase(this->pairArray.begin() + i);
				return true;
			}

			keyInstance--;
		}
	}

	return false;
}

void VDFBlockValue::AddKey(const std::string& key, std::shared_ptr<VDFValue> vdfValue)
{
	Pair pair;
	pair.key = key;
	pair.value = vdfValue;
	this->pairArray.push_back(pair);
}