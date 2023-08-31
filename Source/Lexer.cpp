#include "Lexer.h"
#include "JsonValue.h"

namespace ParseParty
{
	bool operator<(const Lexer::FileLocation& locationA, const Lexer::FileLocation& locationB)
	{
		if (locationA.line == locationB.line)
			return locationA.column < locationB.column;

		return locationA.line < locationB.line;
	}
}

using namespace ParseParty;

//------------------------------- Lexer -------------------------------

Lexer::Lexer()
{
	this->tokenGeneratorList = new std::list<TokenGenerator*>();
}

/*virtual*/ Lexer::~Lexer()
{
	this->Clear();

	delete this->tokenGeneratorList;
}

void Lexer::Clear()
{
	for (TokenGenerator* tokenGenerator : *this->tokenGeneratorList)
		delete tokenGenerator;

	this->tokenGeneratorList->clear();
}

bool Lexer::ReadFile(const std::string& lexiconFile, std::string& error)
{
	bool success = false;
	JsonValue* jsonRootValue = nullptr;

	while (true)
	{
		std::ifstream fileStream;
		fileStream.open(lexiconFile.c_str(), std::ios::in);
		if (!fileStream.is_open())
		{
			error = "Failed to open file: " + lexiconFile;
			break;
		}

		std::stringstream stringStream;
		stringStream << fileStream.rdbuf();
		std::string jsonString = stringStream.str();
		std::string parseError;
		jsonRootValue = JsonValue::ParseJson(jsonString, parseError);
		if (!jsonRootValue)
		{
			error = parseError;
			break;
		}

		const JsonObject* jsonObject = dynamic_cast<JsonObject*>(jsonRootValue);
		if (!jsonObject)
		{
			error = "Expected root-level JSON object.";
			break;
		}

		const JsonArray* jsonTokenGeneratorArray = dynamic_cast<const JsonArray*>(jsonObject->GetValue("token_generators"));
		if (!jsonTokenGeneratorArray)
		{
			error = "Expected to find \"token_generators\" key as an array.";
			break;
		}

		this->Clear();

		for (int i = 0; i < (signed)jsonTokenGeneratorArray->GetSize(); i++)
		{
			const JsonObject* jsonTokenGenerator = dynamic_cast<const JsonObject*>(jsonTokenGeneratorArray->GetValue(i));
			if (!jsonTokenGenerator)
			{
				error = "Each token generator should be an object.";
				break;
			}

			const JsonString* jsonGeneratorName = dynamic_cast<const JsonString*>(jsonTokenGenerator->GetValue("name"));
			if (!jsonGeneratorName)
			{
				error = "No name found for token generator.";
				break;
			}

			const JsonObject* jsonGeneratorConfig = dynamic_cast<const JsonObject*>(jsonTokenGenerator->GetValue("config"));
			if (!jsonGeneratorConfig)
			{
				error = "No config found for token generator \"" + jsonGeneratorName->GetValue() + "\".";
				break;
			}

			TokenGenerator* tokenGenerator = nullptr;

			if (jsonGeneratorName->GetValue() == "ParanTokenGenerator")
				tokenGenerator = new ParanTokenGenerator();
			else if (jsonGeneratorName->GetValue() == "DelimeterTokenGenerator")
				tokenGenerator = new DelimeterTokenGenerator();
			else if (jsonGeneratorName->GetValue() == "NumberTokenGenerator")
				tokenGenerator = new NumberTokenGenerator();
			else if (jsonGeneratorName->GetValue() == "StringTokenGenerator")
				tokenGenerator = new StringTokenGenerator();
			else if (jsonGeneratorName->GetValue() == "OperatorTokenGenerator")
				tokenGenerator = new OperatorTokenGenerator();
			else if (jsonGeneratorName->GetValue() == "IdentifierTokenGenerator")
				tokenGenerator = new IdentifierTokenGenerator();
			else if (jsonGeneratorName->GetValue() == "CommentTokenGenerator")
				tokenGenerator = new CommentTokenGenerator();

			if (!tokenGenerator)
			{
				error = "Unrecognized token generator: " + jsonGeneratorName->GetValue();
				break;
			}

			if (!tokenGenerator->ReadConfig(jsonGeneratorConfig, error))
			{
				delete tokenGenerator;
				break;
			}

			this->tokenGeneratorList->push_back(tokenGenerator);
		}

		if (this->tokenGeneratorList->size() != jsonTokenGeneratorArray->GetSize())
			break;

		success = true;
		break;
	}

	delete jsonRootValue;

	return success;
}

bool Lexer::WriteFile(const std::string& lexiconFile) const
{
	return false;
}

// TODO: We really should have good error reporting here that includes line and column numbers.
bool Lexer::Tokenize(const std::string& codeText, std::vector<Token*>& tokenArray)
{
	if (tokenArray.size() != 0)
		return false;

	const char* codeBuffer = codeText.c_str();
	FileLocation fileLocation{ 1, 1 };

	int i = 0, j = 0;
	while (i < (signed)codeText.length())
	{
		while (i < (signed)codeText.length() && ::isspace(codeText.c_str()[i]))
			i++;

		if (i == codeText.length())
			break;

		while (j < i)
		{
			if (codeBuffer[j++] != '\n')
				fileLocation.column++;
			else
			{
				fileLocation.line++;
				fileLocation.column = 1;
			}
		}

		Token* token = nullptr;

		for (TokenGenerator* tokenGenerator : *this->tokenGeneratorList)
		{
			token = tokenGenerator->GenerateToken(codeBuffer, i);
			if (token)
			{
				token->fileLocation = fileLocation;

				if (token->type == Token::Type::COMMENT)	// TODO: Would we ever care to keep these?
					delete token;
				else
					tokenArray.push_back(token);

				break;
			}
		}

		if (i == j)
			return false;
	}

	return true;
}

//------------------------------- Lexer::Token -------------------------------

Lexer::Token::Token()
{
	this->text = new std::string();
	this->type = Type::UNKNOWN;
	this->fileLocation.line = -1;
	this->fileLocation.column = -1;
}

/*virtual*/ Lexer::Token::~Token()
{
	delete this->text;
}

bool Lexer::Token::IsOpener() const
{
	return this->type == Type::OPEN_CURLY_BRACE || this->type == Type::OPEN_PARAN || this->type == Type::OPEN_SQUARE_BRACKET;
}

bool Lexer::Token::IsCloser() const
{
	return this->type == Type::CLOSE_CURLY_BRACE || this->type == Type::CLOSE_PARAN || this->type == Type::CLOSE_SQUARE_BRACKET;
}

//-------------------------------- Lexer::TokenGenerator --------------------------------

Lexer::TokenGenerator::TokenGenerator()
{
}

/*virtual*/ Lexer::TokenGenerator::~TokenGenerator()
{
}

bool Lexer::TokenGenerator::IsCharFoundIn(char ch, const char* charSet)
{
	int i = 0;
	while (charSet[i] != '\0')
		if (ch == charSet[i++])
			return true;

	return false;
}

//-------------------------------- Lexer::ParanTokenGenerator --------------------------------

Lexer::ParanTokenGenerator::ParanTokenGenerator()
{
}

/*virtual*/ Lexer::ParanTokenGenerator::~ParanTokenGenerator()
{
}

/*virtual*/ Lexer::Token* Lexer::ParanTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	Token* token = nullptr;

	if (codeBuffer[i] == '(')
	{
		token = new Token();
		token->type = Token::Type::OPEN_PARAN;
	}
	else if (codeBuffer[i] == ')')
	{
		token = new Token();
		token->type = Token::Type::CLOSE_PARAN;
	}
	else if (codeBuffer[i] == '[')
	{
		token = new Token();
		token->type = Token::Type::OPEN_SQUARE_BRACKET;
	}
	else if (codeBuffer[i] == ']')
	{
		token = new Token();
		token->type = Token::Type::CLOSE_SQUARE_BRACKET;
	}
	else if (codeBuffer[i] == '{')
	{
		token = new Token();
		token->type = Token::Type::OPEN_CURLY_BRACE;
	}
	else if(codeBuffer[i] == '}')
	{
		token = new Token();
		token->type = Token::Type::CLOSE_CURLY_BRACE;
	}

	if (token)
		*token->text = codeBuffer[i++];

	return token;
}

/*virtual*/ bool Lexer::ParanTokenGenerator::ReadConfig(const JsonObject* jsonConfig, std::string& error)
{
	return true;
}

/*virtual*/ bool Lexer::ParanTokenGenerator::WriteConfig(JsonObject* jsonConfig) const
{
	return false;
}

//-------------------------------- Lexer::DelimeterTokenGenerator --------------------------------

Lexer::DelimeterTokenGenerator::DelimeterTokenGenerator()
{
}

/*virtual*/ Lexer::DelimeterTokenGenerator::~DelimeterTokenGenerator()
{
}

/*virtual*/ Lexer::Token* Lexer::DelimeterTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	Token* token = nullptr;

	if (codeBuffer[i] == ',')
	{
		token = new Token();
		token->type = Token::Type::DELIMETER_COMMA;
	}
	else if (codeBuffer[i] == ';')
	{
		token = new Token();
		token->type = Token::Type::DELIMETER_SEMI_COLON;
	}
	else if (codeBuffer[i] == ':')
	{
		token = new Token();
		token->type = Token::Type::DELIMETER_COLON;
	}

	if (token)
		*token->text = codeBuffer[i++];

	return token;
}

/*virtual*/ bool Lexer::DelimeterTokenGenerator::ReadConfig(const JsonObject* jsonConfig, std::string& error)
{
	return true;
}

/*virtual*/ bool Lexer::DelimeterTokenGenerator::WriteConfig(JsonObject* jsonConfig) const
{
	return false;
}

//-------------------------------- Lexer::StringTokenGenerator --------------------------------

Lexer::StringTokenGenerator::StringTokenGenerator()
{
	this->processEscapeSequences = false;
}

/*virtual*/ Lexer::StringTokenGenerator::~StringTokenGenerator()
{
}

/*virtual*/ Lexer::Token* Lexer::StringTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	if (codeBuffer[i] != '"')
		return nullptr;

	Token* token = new Token();
	token->type = Token::Type::STRING_LITERAL;

	int j = i + 1;
	while (codeBuffer[j] != '\0' && codeBuffer[j] != '"')
		*token->text += codeBuffer[j++];

	if (this->processEscapeSequences && !this->CollapseEscapeSequences(*token->text))
	{
		delete token;
		token = nullptr;
	}
	else if (codeBuffer[j] == '"')
		i = j + 1;
	else
	{
		delete token;
		token = nullptr;
	}

	return token;
}

bool Lexer::StringTokenGenerator::CollapseEscapeSequences(std::string& text)
{
	std::string modifiedText;

	for (int i = 0; text.c_str()[i] != '\0'; i++)
	{
		char ch = text.c_str()[i];
		if (ch != '\\')
			modifiedText += ch;
		else
		{
			char nextCh = text.c_str()[++i];
			if (nextCh == '\0')
				return false;

			switch (nextCh)
			{
			case 't':
				modifiedText += '\t';
				break;
			case 'n':
				modifiedText += '\n';
				break;
			case 'r':
				modifiedText += '\r';
				break;
			case '"':
				modifiedText += '"';
				break;
			case '\\':
				modifiedText += '\\';
				break;
			default:
				modifiedText += '\\';
				i--;
				break;
			}
		}
	}

	text = modifiedText;
	return true;
}

/*virtual*/ bool Lexer::StringTokenGenerator::ReadConfig(const JsonObject* jsonConfig, std::string& error)
{
	const JsonBool* jsonProcessEscapes = dynamic_cast<const JsonBool*>(jsonConfig->GetValue("process_escape_sequences"));
	if (jsonProcessEscapes)
		this->processEscapeSequences = jsonProcessEscapes->GetValue();

	return true;
}

/*virtual*/ bool Lexer::StringTokenGenerator::WriteConfig(JsonObject* jsonConfig) const
{
	return false;
}

//-------------------------------- Lexer::NumberTokenGenerator --------------------------------

Lexer::NumberTokenGenerator::NumberTokenGenerator()
{
}

/*virtual*/ Lexer::NumberTokenGenerator::~NumberTokenGenerator()
{
}

/*virtual*/ Lexer::Token* Lexer::NumberTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	Token* token = nullptr;

	if (codeBuffer[i] == '-' || ::isdigit(codeBuffer[i]))
	{
		token = new Token();
		token->type = Token::Type::NUMBER_LITERAL_INT;

		int j = i;

		if (codeBuffer[i] == '-')
		{
			*token->text += codeBuffer[i];
			j++;
		}

		while (codeBuffer[j] != '\0' && (codeBuffer[j] == '.' || ::isdigit(codeBuffer[j])))
		{
			*token->text += codeBuffer[j];
			
			if (codeBuffer[j] == '.')
				token->type = Token::Type::NUMBER_LITERAL_FLOAT;

			j++;
		}

		if (*token->text == "-")
		{
			delete token;
			token = nullptr;
		}
		else
			i = j;
	}

	return token;
}

/*virtual*/ bool Lexer::NumberTokenGenerator::ReadConfig(const JsonObject* jsonConfig, std::string& error)
{
	return true;
}

/*virtual*/ bool Lexer::NumberTokenGenerator::WriteConfig(JsonObject* jsonConfig) const
{
	return false;
}

//-------------------------------- Lexer::OperatorTokenGenerator --------------------------------

Lexer::OperatorTokenGenerator::OperatorTokenGenerator()
{
	this->operatorSet = new std::set<std::string>();
}

/*virtual*/ Lexer::OperatorTokenGenerator::~OperatorTokenGenerator()
{
	delete this->operatorSet;
}

/*virtual*/ Lexer::Token* Lexer::OperatorTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	const char charSet[] = ".=+-*/%:<>&|!";		// TODO: Should really glean this from the operator set.
	if (!this->IsCharFoundIn(codeBuffer[i], charSet))
		return nullptr;

	Token* token = new Token();
	token->type = Token::Type::OPERATOR;

	int j = i;
	while (this->IsCharFoundIn(codeBuffer[j], charSet))
	{
		*token->text += codeBuffer[j++];
		if (this->operatorSet->find(*token->text) != this->operatorSet->end())
			break;
	}

	if (this->operatorSet->find(*token->text) != this->operatorSet->end())
		i = j;
	else
	{
		delete token;
		token = nullptr;
	}

	return token;
}

/*virtual*/ bool Lexer::OperatorTokenGenerator::ReadConfig(const JsonObject* jsonConfig, std::string& error)
{
	const JsonArray* jsonOperatorArray = dynamic_cast<const JsonArray*>(jsonConfig->GetValue("operators"));
	if (jsonOperatorArray)
	{
		this->operatorSet->clear();

		for (int i = 0; i < (signed)jsonOperatorArray->GetSize(); i++)
		{
			const JsonString* jsonOperatorString = dynamic_cast<const JsonString*>(jsonOperatorArray->GetValue(i));
			if (!jsonOperatorString)
			{
				error = "Expected operator entry to be a string.";
				return false;
			}

			this->operatorSet->insert(jsonOperatorString->GetValue());
		}
	}

	return true;
}

/*virtual*/ bool Lexer::OperatorTokenGenerator::WriteConfig(JsonObject* jsonConfig) const
{
	return false;
}

//-------------------------------- Lexer::IdentifierTokenGenerator --------------------------------

Lexer::IdentifierTokenGenerator::IdentifierTokenGenerator()
{
}

/*virtual*/ Lexer::IdentifierTokenGenerator::~IdentifierTokenGenerator()
{
}

/*virtual*/ Lexer::Token* Lexer::IdentifierTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	if (!::isalpha(codeBuffer[i]))
		return nullptr;

	Token* token = new Token();
	token->type = Token::Type::IDENTIFIER;

	while (::isalpha(codeBuffer[i]) || ::isdigit(codeBuffer[i]) || codeBuffer[i] == '_')
		*token->text += codeBuffer[i++];

	return token;
}

/*virtual*/ bool Lexer::IdentifierTokenGenerator::ReadConfig(const JsonObject* jsonConfig, std::string& error)
{
	// TODO: Maybe configure which identifiers are keywords?
	return true;
}

/*virtual*/ bool Lexer::IdentifierTokenGenerator::WriteConfig(JsonObject* jsonConfig) const
{
	return false;
}

//-------------------------------- Lexer::CommentTokenGenerator --------------------------------

Lexer::CommentTokenGenerator::CommentTokenGenerator()
{
}

/*virtual*/ Lexer::CommentTokenGenerator::~CommentTokenGenerator()
{
}

/*virtual*/ Lexer::Token* Lexer::CommentTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	if (codeBuffer[i] != '#')
		return nullptr;

	Token* token = new Token();
	token->type = Token::Type::COMMENT;

	while (codeBuffer[i] != '\n')
		*token->text += codeBuffer[i++];

	return token;
}

/*virtual*/ bool Lexer::CommentTokenGenerator::ReadConfig(const JsonObject* jsonConfig, std::string& error)
{
	return true;
}

/*virtual*/ bool Lexer::CommentTokenGenerator::WriteConfig(JsonObject* jsonConfig) const
{
	return false;
}