#include "Lexer.h"

using namespace ParseParty;

//------------------------------- Lexer -------------------------------

Lexer::Lexer()
{
	this->tokenGeneratorList = new std::list<TokenGenerator*>();

	this->tokenGeneratorList->push_back(new ParanTokenGenerator());
	this->tokenGeneratorList->push_back(new DelimeterTokenGenerator());
	this->tokenGeneratorList->push_back(new StringTokenGenerator());
	this->tokenGeneratorList->push_back(new OperatorTokenGenerator());
	this->tokenGeneratorList->push_back(new NumberTokenGenerator());
	this->tokenGeneratorList->push_back(new IdentifierTokenGenerator());
}

/*virtual*/ Lexer::~Lexer()
{
	for (TokenGenerator* tokenGenerator : *this->tokenGeneratorList)
		delete tokenGenerator;

	delete this->tokenGeneratorList;
}

bool Lexer::Tokenize(const std::string& codeText, std::vector<Token*>& tokenArray)
{
	if (tokenArray.size() != 0)
		return false;

	const char* codeBuffer = codeText.c_str();
	FileLocation fileLocation{ 0, 0 };

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
				fileLocation.column = 0;
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
		token->type = Token::Type::CLOSE_CURCLY_BRACE;
	}

	if (token)
		*token->text = codeBuffer[i++];

	return token;
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

//-------------------------------- Lexer::StringTokenGenerator --------------------------------

Lexer::StringTokenGenerator::StringTokenGenerator()
{
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

	// TODO: What about "\""?
	int j = i + 1;
	while (codeBuffer[j] != '\0' && codeBuffer[j] != '"')
		*token->text += codeBuffer[j++];

	if (codeBuffer[j] == '"')
		i = j + 1;
	else
	{
		delete token;
		token = nullptr;
	}

	return token;
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

//-------------------------------- Lexer::OperatorTokenGenerator --------------------------------

Lexer::OperatorTokenGenerator::OperatorTokenGenerator()
{
	this->operatorSet = new std::set<std::string>();

	this->operatorSet->insert(".");
	this->operatorSet->insert("=");
	this->operatorSet->insert("+=");
	this->operatorSet->insert("-=");
	this->operatorSet->insert("*=");
	this->operatorSet->insert("/=");
	this->operatorSet->insert("%=");
	this->operatorSet->insert("==");
	this->operatorSet->insert("+");
	this->operatorSet->insert("-");
	this->operatorSet->insert("*");
	this->operatorSet->insert("/");
	this->operatorSet->insert("%");
	this->operatorSet->insert(":");
	this->operatorSet->insert("<");
	this->operatorSet->insert("<=");
	this->operatorSet->insert(">");
	this->operatorSet->insert(">=");
	this->operatorSet->insert("&&");
	this->operatorSet->insert("||");
	this->operatorSet->insert("!");
}

/*virtual*/ Lexer::OperatorTokenGenerator::~OperatorTokenGenerator()
{
	delete this->operatorSet;
}

/*virtual*/ Lexer::Token* Lexer::OperatorTokenGenerator::GenerateToken(const char* codeBuffer, int& i)
{
	const char charSet[] = ".=+-*/%:<>&|!";
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

	while (::isalpha(codeBuffer[i]) || ::isdigit(codeBuffer[i]))
		*token->text += codeBuffer[i++];

	return token;
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