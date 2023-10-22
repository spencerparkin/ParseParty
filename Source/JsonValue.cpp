#include "JsonValue.h"
#include "StringTransformer.h"

using namespace ParseParty;

//-------------------------------- JsonValue --------------------------------

JsonValue::JsonValue()
{
}

/*virtual*/ JsonValue::~JsonValue()
{
}

/*static*/ JsonValue* JsonValue::ParseJson(const std::string& jsonString, std::string& parseError)
{
	Lexer lexer;
	lexer.tokenGeneratorList->push_back(new Lexer::ParanTokenGenerator());
	lexer.tokenGeneratorList->push_back(new Lexer::DelimeterTokenGenerator());
	lexer.tokenGeneratorList->push_back(new Lexer::StringTokenGenerator(true));
	lexer.tokenGeneratorList->push_back(new Lexer::NumberTokenGenerator());
	lexer.tokenGeneratorList->push_back(new Lexer::CommentTokenGenerator());
	lexer.tokenGeneratorList->push_back(new Lexer::IdentifierTokenGenerator());

	std::vector<Lexer::Token*> tokenArray;
	if (!lexer.Tokenize(jsonString, tokenArray, parseError))
		return nullptr;

	if (tokenArray.size() == 0)
	{
		parseError = "Token sequence is size zero.";
		return nullptr;
	}

	JsonValue* jsonValue = ValueFactory(*tokenArray[0]);
	if (!jsonValue)
		parseError = "Could not decypher initial token.";
	else
	{
		int parsePosition = 0;
		if (!jsonValue->ParseTokens(tokenArray, parsePosition, parseError))
		{
			delete jsonValue;
			jsonValue = nullptr;
		}
	}

	for (Lexer::Token* token : tokenArray)
		delete token;

	return jsonValue;
}

/*static*/ JsonValue* JsonValue::ValueFactory(const Lexer::Token& token)
{
	if (token.type == Lexer::Token::Type::OPEN_SQUARE_BRACKET)
		return new JsonArray();
	else if (token.type == Lexer::Token::Type::OPEN_CURLY_BRACE)
		return new JsonObject();
	else if (token.type == Lexer::Token::Type::STRING_LITERAL)
		return new JsonString();
	else if (token.type == Lexer::Token::Type::NUMBER_LITERAL_FLOAT)
		return new JsonFloat();
	else if (token.type == Lexer::Token::Type::NUMBER_LITERAL_INT)
		return new JsonInt();
	else if (token.type == Lexer::Token::Type::IDENTIFIER)
		return new JsonBool();

	return nullptr;
}

/*static*/ std::string JsonValue::MakeTabs(int tabCount)
{
	std::string tabString;
	for (int i = 0; i < tabCount; i++)
		tabString += "\t";
	return tabString;
}

/*static*/ std::string JsonValue::MakeError(const std::vector<Lexer::Token*>& tokenArray, int parsePosition, const std::string& errorMsg)
{
	std::string errorPrefix = "Error: ";
	
	if (0 <= parsePosition && parsePosition < (signed)tokenArray.size())
	{
		const Lexer::FileLocation& fileLocation = tokenArray[parsePosition]->fileLocation;
		errorPrefix += std::format("Line {}, column {}: ", fileLocation.line, fileLocation.column);
	}

	return errorPrefix + errorMsg;
}

//-------------------------------- JsonString --------------------------------

JsonString::JsonString()
{
	this->value = new std::string();
}

JsonString::JsonString(const std::string& value)
{
	this->value = new std::string(value);
}

/*virtual*/ JsonString::~JsonString()
{
	delete this->value;
}

/*virtual*/ bool JsonString::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	EspaceSequenceDecoder decoder;

	std::string stringContents;
	if (!decoder.Transform(*this->value, stringContents))
		return false;

	jsonString += "\"" + stringContents + "\"";
	return true;
}

/*virtual*/ bool JsonString::ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError)
{
	if (parsePosition < 0 || parsePosition >= (signed)tokenArray.size())
	{
		parseError = "Internal error!";
		return false;
	}

	const Lexer::Token* token = tokenArray[parsePosition];

	if (token->type != Lexer::Token::Type::STRING_LITERAL)
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected string literal.");
		return false;
	}

	this->SetValue(*token->text);
	parsePosition++;
	return true;
}

const std::string& JsonString::GetValue() const
{
	return *this->value;
}

void JsonString::SetValue(const std::string& value)
{
	*this->value = value;
}

//-------------------------------- JsonFloat --------------------------------

JsonFloat::JsonFloat()
{
	this->value = 0.0;
}

JsonFloat::JsonFloat(double value)
{
	this->value = value;
}

/*virtual*/ JsonFloat::~JsonFloat()
{
}

/*virtual*/ bool JsonFloat::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	char buffer[128];
	sprintf_s(buffer, sizeof(buffer), "%f", this->value);
	jsonString += buffer;
	return true;
}

/*virtual*/ bool JsonFloat::ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError)
{
	if (parsePosition < 0 || parsePosition >= (signed)tokenArray.size())
	{
		parseError = "Internal error!";
		return false;
	}

	const Lexer::Token* token = tokenArray[parsePosition];

	if (token->type != Lexer::Token::Type::NUMBER_LITERAL_FLOAT)
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected float literal.");
		return false;
	}

	this->SetValue(::atof(token->text->c_str()));
	parsePosition++;
	return true;
}

double JsonFloat::GetValue() const
{
	return value;
}

void JsonFloat::SetValue(double value)
{
	this->value = value;
}

//-------------------------------- JsonInt --------------------------------

JsonInt::JsonInt()
{
	this->value = 0L;
}

JsonInt::JsonInt(long value)
{
	this->value = value;
}

/*virtual*/ JsonInt::~JsonInt()
{
}

/*virtual*/ bool JsonInt::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	char buffer[128];
	sprintf_s(buffer, sizeof(buffer), "%d", this->value);
	jsonString += buffer;
	return true;
}

/*virtual*/ bool JsonInt::ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError)
{
	if (parsePosition < 0 || parsePosition >= (signed)tokenArray.size())
	{
		parseError = "Internal error!";
		return false;
	}

	const Lexer::Token* token = tokenArray[parsePosition];

	if (token->type != Lexer::Token::Type::NUMBER_LITERAL_INT)
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected integer literal.");
		return false;
	}

	this->SetValue(::atoi(token->text->c_str()));
	parsePosition++;
	return true;
}

long JsonInt::GetValue() const
{
	return this->value;
}

void JsonInt::SetValue(long value)
{
	this->value = value;
}

//-------------------------------- JsonObject --------------------------------

JsonObject::JsonObject()
{
	this->valueMap = new JsonValueMap();
}

/*virtual*/ JsonObject::~JsonObject()
{
	this->Clear();

	delete this->valueMap;
}

/*virtual*/ bool JsonObject::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	jsonString += MakeTabs(tabLevel) + "{\n";
	
	int i = 0;

	for (std::pair<std::string, JsonValue*> pair : *this->valueMap)
	{
		if (i++ > 0)
			jsonString += ",\n";

		jsonString += MakeTabs(tabLevel + 1) + "\"" + pair.first + "\":";
		
		if (dynamic_cast<JsonArray*>(pair.second) || dynamic_cast<JsonObject*>(pair.second))
			jsonString += "\n";
		else
			jsonString += " ";

		if (!pair.second->PrintJson(jsonString, tabLevel + 1))
			return false;
	}

	jsonString += "\n" + MakeTabs(tabLevel) + "}";

	return true;
}

/*virtual*/ bool JsonObject::ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError)
{
	if (parsePosition < 0 || parsePosition >= (signed)tokenArray.size())
	{
		parseError = "Internal error!";
		return false;
	}

	const Lexer::Token* token = tokenArray[parsePosition];
	if (token->type != Lexer::Token::Type::OPEN_CURLY_BRACE)
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected open curly brace.");
		return false;
	}

	this->Clear();

	int openCurlyPosition = parsePosition++;

	if (parsePosition >= (signed)tokenArray.size())
	{
		parseError = MakeError(tokenArray, openCurlyPosition, "Run-away curly brace.");
		return false;
	}

	token = tokenArray[parsePosition];
	if (token->type == Lexer::Token::Type::CLOSE_CURLY_BRACE)
	{
		parsePosition++;
		return true;
	}

	while (true)
	{
		token = tokenArray[parsePosition];
		if (token->type != Lexer::Token::Type::STRING_LITERAL)
		{
			parseError = MakeError(tokenArray, parsePosition, "Expected string key.");
			return false;
		}

		std::string key = *token->text;

		if (++parsePosition >= (signed)tokenArray.size())
		{
			parseError = MakeError(tokenArray, openCurlyPosition, "Run-away curly brace.");
			return false;
		}

		token = tokenArray[parsePosition];
		if (token->type != Lexer::Token::Type::DELIMETER_COLON)
		{
			parseError = MakeError(tokenArray, parsePosition, "Expected colon after key.");
			return false;
		}

		if (++parsePosition >= (signed)tokenArray.size())
		{
			parseError = MakeError(tokenArray, openCurlyPosition, "Run-away curly brace.");
			return false;
		}

		token = tokenArray[parsePosition];
		JsonValue* jsonValue = ValueFactory(*token);
		if (!jsonValue)
		{
			parseError = MakeError(tokenArray, parsePosition, "Could not decypher JSON value type.");
			return false;
		}

		if (!this->SetValue(key, jsonValue))
		{
			parseError = "Internal error!";
			delete jsonValue;
			return false;
		}

		if (!jsonValue->ParseTokens(tokenArray, parsePosition, parseError))
			return false;

		if (parsePosition >= (signed)tokenArray.size())
		{
			parseError = MakeError(tokenArray, openCurlyPosition, "Run-away curly brace.");
			return false;
		}

		token = tokenArray[parsePosition];
		if (token->type == Lexer::Token::Type::DELIMETER_COMMA)
			parsePosition++;
		else if (token->type == Lexer::Token::Type::CLOSE_CURLY_BRACE)
		{
			parsePosition++;
			break;
		}
		else
		{
			parseError = MakeError(tokenArray, parsePosition, "Expected comma or close curly brace.");
			return false;
		}
	}

	return true;
}

void JsonObject::Clear()
{
	for (std::pair<std::string, JsonValue*> pair : *this->valueMap)
		delete pair.second;

	this->valueMap->clear();
}

unsigned int JsonObject::GetSize() const
{
	return (int)this->valueMap->size();
}

const JsonValue* JsonObject::GetValue(const std::string& key) const
{
	return const_cast<JsonObject*>(this)->GetValue(key);
}

JsonValue* JsonObject::GetValue(const std::string& key)
{
	JsonValueMap::iterator iter = this->valueMap->find(key);
	if (iter == this->valueMap->end())
		return nullptr;

	return iter->second;
}

bool JsonObject::SetValue(const std::string& key, JsonValue* value)
{
	this->DeleteValue(key);
	this->valueMap->insert(std::pair<std::string, JsonValue*>(key, value));
	return true;
}

bool JsonObject::DeleteValue(const std::string& key)
{
	JsonValueMap::iterator iter = this->valueMap->find(key);
	if (iter == this->valueMap->end())
		return false;

	delete iter->second;
	this->valueMap->erase(iter);
	return true;
}

//-------------------------------- JsonArray --------------------------------

JsonArray::JsonArray()
{
	this->valueArray = new JsonValueArray();
}

JsonArray::JsonArray(const std::vector<double>& floatArray)
{
	this->valueArray = new JsonValueArray();

	for (double value : floatArray)
		this->valueArray->push_back(new JsonFloat(value));
}

JsonArray::JsonArray(const std::vector<int>& intArray)
{
	this->valueArray = new JsonValueArray();

	for (int value : intArray)
		this->valueArray->push_back(new JsonInt(value));
}

/*virtual*/ JsonArray::~JsonArray()
{
	this->Clear();

	delete this->valueArray;
}

/*virtual*/ bool JsonArray::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	jsonString += MakeTabs(tabLevel) + "[\n";

	for (int i = 0; i < (signed)this->valueArray->size(); i++)
	{
		if (i > 0)
			jsonString += ",\n";

		const JsonValue* jsonValue = (*this->valueArray)[i];
		
		if (!(dynamic_cast<const JsonArray*>(jsonValue) || dynamic_cast<const JsonObject*>(jsonValue)))
			jsonString += MakeTabs(tabLevel + 1);

		if (!jsonValue->PrintJson(jsonString, tabLevel + 1))
			return false;
	}

	jsonString += "\n" + MakeTabs(tabLevel) + "]";

	return true;
}

/*virtual*/ bool JsonArray::ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError)
{
	if (parsePosition < 0 || parsePosition >= (signed)tokenArray.size())
	{
		parseError = "Internal error!";
		return false;
	}

	const Lexer::Token* token = tokenArray[parsePosition];
	if (token->type != Lexer::Token::Type::OPEN_SQUARE_BRACKET)
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected open square bracket.");
		return false;
	}

	this->Clear();

	int openBracketPosition = parsePosition++;

	if (parsePosition >= (signed)tokenArray.size())
	{
		parseError = MakeError(tokenArray, openBracketPosition, "Run-away square bracket.");
		return false;
	}

	token = tokenArray[parsePosition];
	if (token->type == Lexer::Token::Type::CLOSE_SQUARE_BRACKET)
	{
		parsePosition++;
		return true;
	}

	while (true)
	{
		token = tokenArray[parsePosition];
		JsonValue* jsonValue = ValueFactory(*token);
		if (!jsonValue)
		{
			parseError = MakeError(tokenArray, parsePosition, "Could not decypher JSON value type.");
			return false;
		}

		this->PushValue(jsonValue);

		if (!jsonValue->ParseTokens(tokenArray, parsePosition, parseError))
			return false;

		if (parsePosition >= (signed)tokenArray.size())
		{
			parseError = MakeError(tokenArray, openBracketPosition, "Run-away square bracket.");
			return false;
		}

		token = tokenArray[parsePosition];
		if (token->type == Lexer::Token::Type::DELIMETER_COMMA)
			parsePosition++;
		else if (token->type == Lexer::Token::Type::CLOSE_SQUARE_BRACKET)
		{
			parsePosition++;
			break;
		}
		else
		{
			parseError = MakeError(tokenArray, parsePosition, "Expected comma or close square bracket.");
			return false;
		}
	}
	
	return true;
}

void JsonArray::Clear()
{
	for (JsonValue* value : *this->valueArray)
		delete value;

	this->valueArray->clear();
}

unsigned int JsonArray::GetSize() const
{
	return (int)this->valueArray->size();
}

const JsonValue* JsonArray::GetValue(unsigned int i) const
{
	return const_cast<JsonArray*>(this)->GetValue(i);
}

JsonValue* JsonArray::GetValue(unsigned int i)
{
	if (i >= this->valueArray->size())
		return nullptr;

	return (*this->valueArray)[i];
}

bool JsonArray::SetValue(unsigned int i, JsonValue* value)
{
	return false;
}

bool JsonArray::RemoveValue(unsigned int i)
{
	return false;
}

bool JsonArray::InsertValue(unsigned int i, JsonValue* value)
{
	return false;
}

void JsonArray::PushValue(JsonValue* value)
{
	this->valueArray->push_back(value);
}

JsonValue* JsonArray::PopValue()
{
	if (this->valueArray->size() == 0)
		return nullptr;

	JsonValue* jsonValue = this->valueArray->back();
	this->valueArray->pop_back();
	return jsonValue;
}

//-------------------------------- JsonBool --------------------------------

JsonBool::JsonBool()
{
	this->value = false;
}

JsonBool::JsonBool(bool value)
{
	this->value = value;
}

/*virtual*/ JsonBool::~JsonBool()
{
}

/*virtual*/ bool JsonBool::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	jsonString += this->value ? "true" : "false";
	return true;
}

/*virtual*/ bool JsonBool::ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError)
{
	if (parsePosition < 0 || parsePosition >= (signed)tokenArray.size())
	{
		parseError = "Internal error!";
		return false;
	}

	const Lexer::Token* token = tokenArray[parsePosition];
	if (token->type != Lexer::Token::Type::IDENTIFIER)
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected identifier.");
		return false;
	}

	if (*token->text == "true")
		this->SetValue(true);
	else if (*token->text == "false")
		this->SetValue(false);
	else
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected identifier to be \"true\" or \"false\".");
		return false;
	}

	parsePosition++;
	return true;
}

bool JsonBool::GetValue() const
{
	return this->value;
}

void JsonBool::SetValue(bool value)
{
	this->value = value;
}

//-------------------------------- JsonNull --------------------------------

JsonNull::JsonNull()
{
}

/*virtual*/ JsonNull::~JsonNull()
{
}

/*virtual*/ bool JsonNull::PrintJson(std::string& jsonString, int tabLevel /*= 0*/) const
{
	jsonString += "null";
	return true;
}

/*virtual*/ bool JsonNull::ParseTokens(const std::vector<Lexer::Token*>& tokenArray, int& parsePosition, std::string& parseError)
{
	if (parsePosition < 0 || parsePosition >= (signed)tokenArray.size())
	{
		parseError = "Internal error!";
		return false;
	}

	const Lexer::Token* token = tokenArray[parsePosition];
	if (token->type != Lexer::Token::Type::IDENTIFIER)
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected identifier");
		return false;
	}

	if (*token->text != "null")
	{
		parseError = MakeError(tokenArray, parsePosition, "Expected identifier to be \"null\".");
		return false;
	}

	parsePosition++;
	return true;
}