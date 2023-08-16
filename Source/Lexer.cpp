#include "Lexer.h"

using namespace ParseParty;

//------------------------------- Lexer -------------------------------

Lexer::Lexer()
{
}

/*virtual*/ Lexer::~Lexer()
{
}

bool Lexer::Tokenize(const std::string& codeText, std::vector<Token*>& tokenArray)
{
	int i = 0;
	while (i < (signed)codeText.length())
	{
		Token* token = new Token();

		if (!token->Eat(codeText.c_str(), i))
		{
			delete token;
			break;
		}

		tokenArray.push_back(token);
	}

	return true;
}

//------------------------------- Lexer::Token -------------------------------

Lexer::Token::Token()
{
	this->text = new std::string();
	this->type = Type::UNKNOWN;
}

/*virtual*/ Lexer::Token::~Token()
{
	delete this->text;
}

bool Lexer::Token::Eat(const char* givenBuffer, int& i)
{
	while (::isspace(givenBuffer[i]))
		i++;

	*this->text = "";

	const char* tokenText = &givenBuffer[i];
	if (tokenText[0] == '"')
	{
		int j = 1;
		while (tokenText[j] != '\0' && tokenText[j] != '"')
		{
			*this->text += tokenText[j];
			j++;
		}

		if (tokenText[j] == '\0')
			return false;

		i += j + 1;
		this->type = Type::STRING_LITERAL;
		return true;
	}
	else if (*tokenText == '-' || ::isdigit(*tokenText))
	{
		this->type = Type::NUMBER_LITERAL_INT;
		int j = 0;
		while (tokenText[j] != '\0' && (tokenText[j] == '-' || tokenText[j] == '.' || ::isdigit(tokenText[j])))
		{
			if (tokenText[j] == '.')
				this->type = Token::Type::NUMBER_LITERAL_FLOAT;

			*this->text += tokenText[j];
			j++;
		}

		i += j;
		return true;
	}
	else if (*tokenText == '{')
	{
		this->type = Type::OPEN_CURLY_BRACE;
		i++;
		return true;
	}
	else if (*tokenText == '}')
	{
		this->type = Type::CLOSE_CURCLY_BRACE;
		i++;
		return true;
	}
	else if (*tokenText == '[')
	{
		this->type = Type::OPEN_SQUARE_BRACKET;
		i++;
		return true;
	}
	else if (*tokenText == ']')
	{
		this->type = Type::CLOSE_SQUARE_BRACKET;
		i++;
		return true;
	}
	else if (*tokenText == ':')
	{
		this->type = Type::COLON;
		i++;
		return true;
	}
	else if (*tokenText == ',')
	{
		this->type = Type::COMMA;
		i++;
		return true;
	}

	return false;
}