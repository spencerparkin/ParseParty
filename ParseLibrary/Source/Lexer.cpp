#include "Lexer.h"
#include "JsonValue.h"
#include "StringTransformer.h"

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
	this->tabSize = 4;
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
	std::ifstream fileStream;
	fileStream.open(lexiconFile.c_str(), std::ios::in);
	if (!fileStream.is_open())
	{
		error = "Failed to open file: " + lexiconFile;
		return false;
	}

	std::stringstream stringStream;
	stringStream << fileStream.rdbuf();
	std::string jsonString = stringStream.str();
	std::string parseError;
	std::shared_ptr<JsonValue> jsonRootValue = JsonValue::ParseJson(jsonString, parseError);
	if (!jsonRootValue)
	{
		error = parseError;
		return false;
	}

	const JsonObject* jsonObject = dynamic_cast<JsonObject*>(jsonRootValue.get());
	if (!jsonObject)
	{
		error = "Expected root-level JSON object.";
		return false;
	}

	const JsonArray* jsonTokenGeneratorArray = dynamic_cast<const JsonArray*>(jsonObject->GetValue("token_generators"));
	if (!jsonTokenGeneratorArray)
	{
		error = "Expected to find \"token_generators\" key as an array.";
		return false;
	}

	this->Clear();

	for (int i = 0; i < (signed)jsonTokenGeneratorArray->GetSize(); i++)
	{
		const JsonObject* jsonTokenGenerator = dynamic_cast<const JsonObject*>(jsonTokenGeneratorArray->GetValue(i));
		if (!jsonTokenGenerator)
		{
			error = "Each token generator should be an object.";
			return false;
		}

		const JsonString* jsonGeneratorName = dynamic_cast<const JsonString*>(jsonTokenGenerator->GetValue("name"));
		if (!jsonGeneratorName)
		{
			error = "No name found for token generator.";
			return false;
		}

		const JsonObject* jsonGeneratorConfig = dynamic_cast<const JsonObject*>(jsonTokenGenerator->GetValue("config"));
		if (!jsonGeneratorConfig)
		{
			error = "No config found for token generator \"" + jsonGeneratorName->GetValue() + "\".";
			return false;
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
			return false;
		}

		if (!tokenGenerator->ReadConfig(jsonGeneratorConfig, error))
		{
			delete tokenGenerator;
			return false;
		}

		this->tokenGeneratorList->push_back(tokenGenerator);
	}

	if (this->tokenGeneratorList->size() != jsonTokenGeneratorArray->GetSize())
		return false;

	return true;
}

bool Lexer::WriteFile(const std::string& lexiconFile) const
{
	return false;
}

bool Lexer::Tokenize(const std::string& codeText, std::vector<std::shared_ptr<Token>>& tokenArray, std::string& error, bool keepComments /*= false*/, FileLocation initialFileLocation /*= FileLocation{ 1, 1 }*/)
{
	if (tokenArray.size() != 0)
		return false;

	const char* codeBuffer = codeText.c_str();
	FileLocation fileLocation = initialFileLocation;

	int i = 0, j = 0;
	while (i < (signed)codeText.length())
	{
		while (i < (signed)codeText.length() && ::isspace(codeText.c_str()[i]))
			i++;

		if (i == codeText.length())
			break;

		while (j < i)
		{
			if (codeBuffer[j] != '\n')
			{
				if (codeBuffer[j] == '\t')
					fileLocation.column += this->tabSize;
				else
					fileLocation.column++;
			}
			else
			{
				fileLocation.line++;
				fileLocation.column = 1;
			}

			j++;
		}

		std::shared_ptr<Token> token;

		for (TokenGenerator* tokenGenerator : *this->tokenGeneratorList)
		{
			token = tokenGenerator->GenerateToken(codeBuffer, i);
			if (token)
			{
				token->fileLocation = fileLocation;

				if (token->type != Token::Type::COMMENT || keepComments)
					tokenArray.push_back(token);

				break;
			}
		}

		if (i == j)
		{
			error = FormatString("Failed to tokenize at line %d, column %d.", fileLocation.line, fileLocation.column);
			return false;
		}
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

//-------------------------------- Lexer::ParanTokenGenerator --------------------------------

Lexer::ParanTokenGenerator::ParanTokenGenerator()
{
}

/*virtual*/ Lexer::ParanTokenGenerator::~ParanTokenGenerator()
{
}

/*virtual*/ std::shared_ptr<Lexer::Token> Lexer::ParanTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	std::shared_ptr<Token> token;

	if (codeBuffer[i] == '(')
	{
		token = std::make_shared<Token>();
		token->type = Token::Type::OPEN_PARAN;
	}
	else if (codeBuffer[i] == ')')
	{
		token = std::make_shared<Token>();
		token->type = Token::Type::CLOSE_PARAN;
	}
	else if (codeBuffer[i] == '[')
	{
		token = std::make_shared<Token>();
		token->type = Token::Type::OPEN_SQUARE_BRACKET;
	}
	else if (codeBuffer[i] == ']')
	{
		token = std::make_shared<Token>();
		token->type = Token::Type::CLOSE_SQUARE_BRACKET;
	}
	else if (codeBuffer[i] == '{')
	{
		token = std::make_shared<Token>();
		token->type = Token::Type::OPEN_CURLY_BRACE;
	}
	else if(codeBuffer[i] == '}')
	{
		token = std::make_shared<Token>();
		token->type = Token::Type::CLOSE_CURLY_BRACE;
	}

	if (token.get())
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

/*virtual*/ std::shared_ptr<Lexer::Token> Lexer::DelimeterTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	std::shared_ptr<Token> token;

	if (codeBuffer[i] == ',')
	{
		token = std::make_shared<Token>();
		token->type = Token::Type::DELIMETER_COMMA;
	}
	else if (codeBuffer[i] == ';')
	{
		token = std::make_shared<Token>();
		token->type = Token::Type::DELIMETER_SEMI_COLON;
	}
	else if (codeBuffer[i] == ':')
	{
		token = std::make_shared<Token>();
		token->type = Token::Type::DELIMETER_COLON;
	}

	if (token.get())
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

Lexer::StringTokenGenerator::StringTokenGenerator(bool processEscapeSequences /*= false*/)
{
	this->processEscapeSequences = processEscapeSequences;
}

/*virtual*/ Lexer::StringTokenGenerator::~StringTokenGenerator()
{
}

/*virtual*/ std::shared_ptr<Lexer::Token> Lexer::StringTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	if (codeBuffer[i] != '"')
		return nullptr;

	std::shared_ptr<Token> token = std::make_shared<Token>();
	token->type = Token::Type::STRING_LITERAL;

	int j = i + 1;
	while (codeBuffer[j] != '\0')
	{
		if (codeBuffer[j] == '"' && (!this->processEscapeSequences || codeBuffer[j - 1] != '\\'))
			break;

		*token->text += codeBuffer[j++];
	}

	if (codeBuffer[j] == '\0' || (this->processEscapeSequences && !this->CollapseEscapeSequences(*token->text)))
		token.reset();
	else
	{
		i = j + 1;
	}

	return token;
}

bool Lexer::StringTokenGenerator::CollapseEscapeSequences(std::string& text)
{
	EspaceSequenceEncoder encoder;

	std::string modifiedText;
	if (!encoder.Transform(text, modifiedText))
		return false;

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

/*virtual*/ std::shared_ptr<Lexer::Token> Lexer::NumberTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	std::shared_ptr<Token> token;

	if (codeBuffer[i] == '-' || ::isdigit(codeBuffer[i]))
	{
		token = std::make_shared<Token>();
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
			token.reset();
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
	this->operatorCharSet = nullptr;
}

/*virtual*/ Lexer::OperatorTokenGenerator::~OperatorTokenGenerator()
{
	delete this->operatorSet;
	delete this->operatorCharSet;
}

/*virtual*/ std::shared_ptr<Lexer::Token> Lexer::OperatorTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	if (!this->operatorCharSet)
	{
		this->operatorCharSet = new std::set<char>();
		for (const std::string& operatorText : *this->operatorSet)
			for (int i = 0; operatorText.c_str()[i] != '\0'; i++)
				if (this->operatorCharSet->find(operatorText.c_str()[i]) == this->operatorCharSet->end())
					this->operatorCharSet->insert(operatorText.c_str()[i]);
	}

	if (this->operatorCharSet->find(codeBuffer[i]) == this->operatorCharSet->end())
		return std::shared_ptr<Token>();

	std::string operatorText;
	std::string chosenOperatorText;

	int j = i;
	while (this->operatorCharSet->find(codeBuffer[j]) != this->operatorCharSet->end())
	{
		operatorText += codeBuffer[j++];
		if (this->operatorSet->find(operatorText) != this->operatorSet->end())
			chosenOperatorText = operatorText;
	}

	if (chosenOperatorText.length() == 0)
		return std::shared_ptr<Token>();

	std::shared_ptr<Token> token = std::make_shared<Token>();
	token->type = Token::Type::OPERATOR;
	*token->text = chosenOperatorText;

	i += (int)token->text->size();

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
	this->keywordSet = new std::set<std::string>();
}

/*virtual*/ Lexer::IdentifierTokenGenerator::~IdentifierTokenGenerator()
{
	delete this->keywordSet;
}

/*virtual*/ std::shared_ptr<Lexer::Token> Lexer::IdentifierTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	if (!::isalpha(codeBuffer[i]))
		return nullptr;

	std::shared_ptr<Token> token = std::make_shared<Token>();
	token->type = Token::Type::IDENTIFIER;

	while (::isalpha(codeBuffer[i]) || ::isdigit(codeBuffer[i]) || codeBuffer[i] == '_')
		*token->text += codeBuffer[i++];

	if (this->keywordSet->find(*token->text) != this->keywordSet->end())
		token->type = Token::Type::IDENTIFIER_KEYWORD;

	return token;
}

/*virtual*/ bool Lexer::IdentifierTokenGenerator::ReadConfig(const JsonObject* jsonConfig, std::string& error)
{
	this->keywordSet->clear();

	const JsonArray* jsonKeywordArray = dynamic_cast<const JsonArray*>(jsonConfig->GetValue("keywords"));
	if (jsonKeywordArray)
	{
		for (int i = 0; i < (signed)jsonKeywordArray->GetSize(); i++)
		{
			const JsonString* jsonKeywordString = dynamic_cast<const JsonString*>(jsonKeywordArray->GetValue(i));
			if (jsonKeywordString)
				this->keywordSet->insert(jsonKeywordString->GetValue());
			else
			{
				error = "Encountered keyword entry that is not a string.";
				return false;
			}
		}
	}

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

/*virtual*/ std::shared_ptr<Lexer::Token> Lexer::CommentTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	if (codeBuffer[i] != '#')
		return std::shared_ptr<Token>();

	std::shared_ptr<Token> token = std::make_shared<Token>();
	token->type = Token::Type::COMMENT;

	while (codeBuffer[i] != '\0' && codeBuffer[i] != '\n')
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